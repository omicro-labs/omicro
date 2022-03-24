#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "omicroclient.h"
#include "omutil.h"
#include "ommsghdr.h"

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
	tv.tv_sec = 30;  /* 10 Secs Timeout */
	tv.tv_usec = 0; 
	setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv, sizeof(struct timeval));
	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv, sizeof(struct timeval));

	d("a70231 OmicroClient ctor connectOK_ srv=%s port=%d", srv, port);
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		d("a72231 dtor of OmicroClient client_.stop()");
		::close( socket_);
	}
}

sstr OmicroClient::sendMessage( const sstr &msg, bool expectReply )
{
	d("a4421 OmicroClient::sendMessage(%s) expectReply=%d", s(msg), expectReply );

	if ( ! connectOK_ ) {
		d("a52031 sendMessage return empty because connect %s:%d is not OK", s(srv_), port_);
		return "";
	}

	char hdr[OMHDR_SZ+1];
	memset(hdr, 0, OMHDR_SZ+1);
	OmMsgHdr mhdr(hdr, OMHDR_SZ);
	mhdr.setLength(msg.size());
	mhdr.setPlain();

	long len1 = safewrite(socket_, hdr, OMHDR_SZ );
	if ( len1 < 0 ) {
		d("a4202 OmicroClient::sendMessage %s:%d write timeout empty hdr", s(srv_), port_);
		return "";
	}

	d("a23373 client write hdr len1=%d",len1);
	long len2 = safewrite(socket_, msg.c_str(), msg.size() );
	if ( len2 < 0 ) {
		d("a4203 OmicroClient::sendMessage write timeout empty msg");
		return "";
	}

	d("a23373 client write data len2=%d expectReply=%d", len2, expectReply);

	if ( expectReply ) {
		// sleep(1);
		char hdr2[OMHDR_SZ+1];
		memset(hdr2, 0, OMHDR_SZ+1);

    	long hdrlen = saferead(socket_, hdr2, OMHDR_SZ );
		if ( hdrlen < 0 ) {
			d("a4205 OmicroClient::sendMessage read %s:%d timeout hdr2 empty []", s(srv_), port_);
			return "";
		}

		OmMsgHdr mhdr2(hdr2, OMHDR_SZ);
		ulong sz = mhdr2.getLength();
		d("a23373 client get hdr hdrlen=%d hdr2=[%s] sz=%d", hdrlen, hdr2, sz );

		char *reply = (char*)malloc(sz+1);
    	long reply_length = saferead(socket_, reply, sz );
		if ( reply_length < 0 ) {
			d("a4206 OmicroClient::sendMessage read timeout reply empty []");
			free( reply );
			return "";
		}

		sstr res(reply, sz);
		d("client Reply is: hdrlen=%d reply_length=%d sz=%d", hdrlen, reply_length, sz);
		d("       reply=[%s]", s(res) );

		free(reply);
		return res;
	}

    return "";
}
