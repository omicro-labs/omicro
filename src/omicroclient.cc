#include <iostream>
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

    addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int gAddRes = getaddrinfo(ipAddress, portNum, &hints, &p);
    if (gAddRes != 0) {
        std::cout << gai_strerror(gAddRes) << "\n";
        return;
    }

    if (p == NULL) {
        std::cout << "No addresses found\n";
        return;
    }

    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
        std::cout << "Error while creating socket\n";
        std::cout << strerror(errno) << "\n";
        std::cout << errno << "\n";
        return;
    }

	d("a4088 connect to [%s] [%d] ...", srv, port );
    int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
    if (connectR == -1) {
        close(sockFD);
        std::cout << "Error while connecting socket\n";
        std::cout << errno << " " << strerror(errno) << "\n";
        return;
    }

	socket_ = sockFD;

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
	if ( ! connectOK_ ) {
		d("a52031 sendMessage return empty because connect is not OK");
		return "";
	}

	char hdr[OMHDR_SZ+1];
	memset(hdr, 0, OMHDR_SZ+1);
	OmMsgHdr mhdr(hdr, OMHDR_SZ);
	mhdr.setLength(msg.size());
	mhdr.setPlain();

	int len1 = safewrite(socket_, hdr, OMHDR_SZ );
	std::cout << "a23373 client write hdr len1=" << len1 << std::endl;
	int len2 = safewrite(socket_, msg.c_str(), msg.size() );
	std::cout << "a23373 client write data len2=" << len2 << std::endl;

	if ( expectReply ) {
		char hdr2[OMHDR_SZ+1];
		memset(hdr2, 0, OMHDR_SZ+1);

    	size_t hdrlen = saferead(socket_, hdr2,OMHDR_SZ );

		OmMsgHdr mhdr2(hdr2, OMHDR_SZ);
		ulong sz = mhdr2.getLength();
		std::cout << "a23373 client get hdr hdrlen=" << hdrlen << std::endl;
		std::cout << "a23373 client get data sz=" << sz << std::endl;

		char *reply = (char*)malloc(sz+1);
		memset(reply, 0, sz+1);

    	size_t reply_length = saferead(socket_, reply, sz );
    	std::cout << "client Reply is: " << hdrlen << " " << reply_length << " " << sz << " [";
    	std::cout.write(reply, reply_length);
    	std::cout << "]\n";

		sstr res(reply, sz);
		free(reply);
		return res;
	}

    return "";
}
