#include <cstring>
#include <netinet/tcp.h>
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
	srv_ = sstr(srv);
	port_ = port;

    addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int gAddRes = getaddrinfo(srv, ps, &hints, &p);
    if (gAddRes != 0) {
		i("E20031 getaddrinfo error %s", gai_strerror(gAddRes) );
        return;
    }

    if (p == NULL) {
        i("E20034 No addresses found %s:%d", srv, port);
        return;
    }

    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
		i("E20035 Error creating socket errno=%d errstr=[%s]", errno, strerror(errno) ); 
        return;
    }

	d("a4088 connect to %s:%d ...", srv, port );
	int tries = 0;
	while ( true ) {
    	int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
		if ( connectR != -1 ) {
			break;
		}

		++ tries;
		i("a00383 connecting %s:%d error, tries=%d retry ...", srv, port, tries);
		sleep(1);

    	if ( tries >= 3) {
        	close(sockFD);
			i("E20036 Error connect %s:%d errno=%d errstr=[%s]", srv, port, errno, strerror(errno) ); 
        	return;
    	}
	}

	socket_ = sockFD;
	struct timeval tv;
	tv.tv_sec = 60;
	tv.tv_usec = 0; 
	setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv, sizeof(struct timeval));
	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv, sizeof(struct timeval));

	int yes = 1;
	setsockopt( socket_, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(yes));

	d("a70231 OmicroClient ctor connectOK_ srv=%s port=%d", srv, port);
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		//d("a72231 dtor of OmicroClient client_.stop() srv=%s port=%d", s(srv_), port_ );
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

	const std::lock_guard<std::mutex> lock(mutex_);

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

sstr OmicroClient::sendTrxn( OmicroTrxn &t )
{
	t.setInitTrxn();
	sstr alldata; t.allstr(alldata);

	sstr reply = sendMessage( OM_TXN, alldata, true );
	return reply;
}

sstr OmicroClient::sendQuery( OmicroTrxn &t )
{
	t.setInitTrxn();
	sstr alldata; t.allstr(alldata);
	sstr reply = sendMessage( OM_XNQ, alldata, true );
	if ( reply.size() < 1 ) {
		d("a11123 sendMessage empty");
		return "";
	}
	d("a344409 sendQuery sendMessage reply=[%s]", reply.c_str() );

	OmResponse resp( reply );
	sstr trxnId = resp.TID_;
	if ( resp.STT_ == OM_RESP_ERR ) {
		d("a322082 got OM_RESP_ERR from server [%s]", resp.RSN_.c_str() );
		return resp.RSN_;
	}

	sleep(5);
	OmicroSimpleQuery q;
	q.setTrxnId( trxnId );
	q.setSender( t.sender_ );
	q.setTimeStamp( t.timestamp_ );
	sstr data; q.str("QQ", data);

	reply = sendMessage( OM_RQ, data, true );
	return reply;
}

sstr OmicroClient::reqPublicKey( int waitSeconds)
{
	OmicroSimpleQuery q;
	q.setTrxnId( "1" );

	int waitCnt = 30;
	int cnt = 0;
	sstr cmd, reply;

	q.strGetPublicKey( cmd );
	d("a33502 q.strGetPublicKey cmd=[%s]", s(cmd) );

	while ( true ) {
		reply = sendMessage( OM_RQ, cmd, true );
		if ( reply.size() < 1 ) {
			break;
		}

		OmResponse resp( reply );
		if ( resp.STT_ == OM_RESP_OK ) {
			d("a32038 recved pubkey=[%s]", s(resp.DAT_) );
			return resp.DAT_;
		}

		usleep(1000*100);
		++cnt;
		if ( cnt > waitCnt ) {
			break;
		}
	}

	return "INVALID248";
}
