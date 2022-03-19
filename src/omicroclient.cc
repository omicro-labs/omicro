#include <iostream>
#include "omicroclient.h"
#include "omutil.h"
#include "ommsghdr.h"

EXTERN_LOGGING

OmicroClient::OmicroClient( const char *srv, int port )
{
	char ps[32];
	sprintf(ps, "%d", port);

	connectOK_ = false;

    socket_ = new tcp::socket(io_context_);

    tcp::resolver resolver(io_context_);
	d("a4088 connect to [%s] [%d] ...", srv, port );
	try {
    	boost::asio::connect(*socket_, resolver.resolve(srv, ps ));
	} catch (std::exception& e) {
		d("a70231 OmicroClient ctor connect error srv=%s port=%d err=[%s]", srv, port, e.what());
		return;
	} catch ( ... ) {
		d("a70231 OmicroClient ctor connect unknown error srv=%s port=%d", srv, port );
		return;
	}
    //socket_->connect( tcp::endpoint( boost::asio::ip::address::from_string(srv), port ));

	d("a70231 OmicroClient ctor connectOK_ srv=%s port=%d", srv, port);
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		d("a72231 dtor of OmicroClient client_.stop()");
		socket_->close();
	}
	delete socket_;
}

sstr OmicroClient::sendMessage( const sstr &msg, bool expectReply )
{
	if ( ! connectOK_ ) {
		d("a52031 sendMessage return empty because connect is not OK");
		return "";
	}

	char hdr[OMHDR_SZ];
	OmMsgHdr mhdr(hdr, OMHDR_SZ);
	mhdr.setLength(msg.size());
	mhdr.setPlain();

	boost::asio::write(*socket_, boost::asio::buffer(hdr, OMHDR_SZ) );
	boost::asio::write(*socket_, boost::asio::buffer(msg.c_str(), msg.size()) );

	if ( expectReply ) {
    	size_t hdrlen = boost::asio::read(*socket_, boost::asio::buffer(hdr, OMHDR_SZ));
		char hdr2[OMHDR_SZ];
		OmMsgHdr mhdr2(hdr2, OMHDR_SZ);
		ulong sz = mhdr2.getLength();

		char *reply = (char*)malloc(sz);

    	size_t reply_length = boost::asio::read(*socket_, boost::asio::buffer(reply, sz));
    	std::cout << "Reply is: " << hdrlen << " " << reply_length << " " << sz << " ";
    	std::cout.write(reply, reply_length);
    	std::cout << "\n";

		free(reply);

	}
    return "";
}
