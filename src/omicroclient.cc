#include <cstring>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/bind/bind.hpp>

#include "omicroclient.h"
#include "omicrotrxn.h"
#include "omicroquery.h"
#include "omutil.h"
#include "ommsghdr.h"
#include "omicrodef.h" 
#include "omlog.h" 
#include "omresponse.h" 

EXTERN_LOGGING

OmicroClient::OmicroClient( const char *srv, int port )
{
	char ps[32];
	sprintf(ps, "%d", port);

	connectOK_ = false;

    auto &ipAddress = srv;
    auto &portNum   = ps;
	srv_ = srv;
	port_ = port;

    addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int gAddRes = getaddrinfo(ipAddress, portNum, &hints, &p);
    if (gAddRes != 0) {
		i("E20031 getaddrinfo error %s", gai_strerror(gAddRes) );
        return;
    }

    if (p == NULL) {
        i("E20034 No addresses found");
        return;
    }

    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
		i("E20035 Error creating socket errno=%d errstr=[%s]", errno, strerror(errno) ); 
        return;
    }

	d("a4088 connect to [%s] [%d] ...", srv, port );
    int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
    if (connectR == -1) {
        close(sockFD);
		i("E20036 Error connect errno=%d errstr=[%s]", errno, strerror(errno) ); 
        return;
    }

	socket_ = sockFD;
	struct timeval tv;
	tv.tv_sec = 5;  // 10 Secs Timeout
	tv.tv_usec = 0; 
	//setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv, sizeof(struct timeval));
	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv, sizeof(struct timeval));

	d("a70231 OmicroClient ctor connectOK_ srv=%s port=%d", srv, port);
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		d("a72231 dtor of OmicroClient client_.stop() srv=%s port=%d", s(srv_), port_ );
		::close( socket_);
	}
}

sstr OmicroClient::sendMessage( char mtype, const sstr &msg, bool expectReply )
{
	//d("a4421 OmicroClient::sendMessage(msg=%s) msglen=%d expectReply=%d", s(msg), msg.size(), expectReply );
	d("a4421 OmicroClient::sendMessage expectReply=%d", expectReply );

	if ( ! connectOK_ ) {
		d("a52031 sendMessage return empty because connect %s:%d is not OK", s(srv_), port_);
		return "";
	}

	char hdr[OMHDR_SZ+1];
	OmMsgHdr mhdr(hdr, OMHDR_SZ, true);
	mhdr.setLength(msg.size());
	mhdr.setPlain();
	mhdr.setMsgType( mtype );
	hdr[OMHDR_SZ] = '\0';
	//d("a10192 hdr=[%s] OMHDR_SZ=%d msg.size()=%d safewrte ...", hdr, OMHDR_SZ, msg.size() );

	long len1 = safewrite(socket_, hdr, OMHDR_SZ );
	if ( len1 <= 0 ) {
		d("a4202 OmicroClient::sendMessage %s:%d write timeout empty hdr", s(srv_), port_);
		return "";
	}
	//d("a23373 client write hdr len1=%d",len1);

	long len2 = safewrite(socket_, msg.c_str(), msg.size() );
	if ( len2 <= 0 ) {
		d("a4203 OmicroClient::sendMessage write timeout empty msg");
		return "";
	}
	//d("a23373 client write data len2=%d expectReply=%d", len2, expectReply);

	int cnt = 0;
	sstr res;
	if ( expectReply ) {
		//sleep(3);
		char hdr2[OMHDR_SZ+1];
		OmMsgHdr mhdr2(hdr2, OMHDR_SZ, true);

    	long hdrlen = saferead(socket_, hdr2, OMHDR_SZ );
		if ( hdrlen <= 0 ) {
			i("E333928 OmicroClient::sendMessage read from %s:%d timeout hdr2 empty hdrlen=%d []", s(srv_), port_, hdrlen);
			i("errno=%d errstr=[%s]", errno, strerror(errno) );
			return "";
		}

		ulong sz = mhdr2.getLength();
		//d("a23373 client received hdr hdrlen=%d hdr2=[%s] sz=%d", hdrlen, hdr2, sz );

		char *reply = (char*)malloc(sz+1);
    	long reply_length = saferead(socket_, reply, sz );
		if ( reply_length <= 0 ) {
			i("E30298 OmicroClient::sendMessage read timeout reply empty reply_length=%d []", reply_length);
			free( reply );
			return "";
		}

		res = sstr(reply, sz);
		//d("cnt=%d client received data: hdrlen=%d reply_length=%d sz=%d", cnt, hdrlen, reply_length, sz);
		//d("cnt=%d       received data =[%s]", cnt, s(res) );

		free(reply);
		++cnt;
	}

	return res;
}

sstr OmicroClient::sendTrxn( OmicroTrxn &t, int waitSeconds)
{
	t.setInitTrxn();
	sstr alldata; t.allstr(alldata);

	sstr reply = sendMessage( OM_TXN, alldata, true );
	d("14073 sendTrxn sendMessage reply=[%s]", s(reply));

	OmResponse resp( reply.c_str() );
	d("a42128 sendTrxn reply=[%s]\n", s(reply));
	sstr trxnId = resp.TID_;

	if ( resp.STT_ == OM_RESP_ERR ) {
		d("a32208 got INVALID from server [%s]", reply.c_str() );
		return reply;
	}

	OmicroSimpleQuery q;
	q.setTrxnId( trxnId );
	q.setSender( t.sender_ );
	q.setTimeStamp( t.timestamp_ );

	int WAIT_MS = 50;
	int waitCnt = waitSeconds*(1000/WAIT_MS);
	int cnt = 0;

	sstr data; q.str("QT", data);

	while ( true ) {
		reply = sendMessage( OM_RQ, data, true );
		OmResponse resp( reply.c_str() );
		d("a423376 sendMessage OM_RQ reply=[%s]", s(reply) );
		if ( resp.RSN_ == "FAILED" ) {
			break;
		}

		if ( resp.RSN_ != "NOTFOUND" ) {
			break;
		}

		usleep(1000*WAIT_MS);
		++cnt;
		if ( cnt > waitCnt ) {
			break;
		}
	}

	return reply;
}

sstr OmicroClient::sendQuery( OmicroTrxn &t, int waitSeconds )
{
	t.setInitTrxn();
	sstr alldata; t.allstr(alldata);
	sstr reply = sendMessage( OM_XNQ, alldata, true );
	d("a344409 sendQuery sendMessage reply=[%s]", reply.c_str() );

	OmResponse resp( reply.c_str() );
	sstr trxnId = resp.TID_;
	if ( resp.STT_ == OM_RESP_ERR ) {
		d("a32208 got INVALID from server [%s]", resp.RSN_.c_str() );
		return resp.RSN_;
	}

	OmicroSimpleQuery q;
	q.setTrxnId( trxnId );
	q.setSender( t.sender_ );
	q.setTimeStamp( t.timestamp_ );

	int WAIT_MS = 50;
	int waitCnt = waitSeconds*(1000/WAIT_MS);
	int cnt = 0;
	sstr data; q.str("QQ", data);

	while ( true ) {
		reply = sendMessage( OM_RQ, data, true );
		d("a73714 sendMessage got reply=[%s] ", reply.c_str() );

		OmResponse resp( reply.c_str() );
		if ( resp.RSN_ == "FAILED" ) {
			break;
		}

		if ( resp.RSN_ != "NOTFOUND" ) {
			break;
		}

		usleep(1000*WAIT_MS);
		++cnt;
		if ( cnt > waitCnt ) {
			break;
		}
	}

	return reply;
}

sstr OmicroClient::reqPublicKey( int waitSeconds)
{
	OmicroSimpleQuery q;
	q.setTrxnId( "1" );

	int WAIT_MS = 50;
	int waitCnt = waitSeconds*(1000/WAIT_MS);
	int cnt = 0;
	sstr cmd, reply;

	q.strGetPublicKey( cmd );

	while ( true ) {
		reply = sendMessage( OM_RQ, cmd, true );
		OmResponse resp( reply.c_str() );
		if ( resp.STT_ == OM_RESP_OK ) {
			// d("a32038 recved pubkey=[%s]", s(resp.DAT_) );
			return resp.DAT_;
		}

		usleep(1000*WAIT_MS);
		++cnt;
		if ( cnt > waitCnt ) {
			break;
		}
	}

	return "INVALID248";
}
