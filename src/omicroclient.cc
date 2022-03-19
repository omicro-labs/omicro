#include <iostream>
#include "omicroclient.h"
#include "omutil.h"
EXTERN_LOGGING

OmicroClient::OmicroClient( const char *srv, int port )
{
	char ps[32];
	sprintf(ps, "%d", port);

	connectOK_ = false;

    socket_ = new tcp::socket(io_context_);

    // tcp::resolver resolver(io_context_);
    //boost::asio::connect(*socket_, resolver.resolve(srv, ps ));
	d("a4088 connect to [%s] [%d] ...", srv, port );
    socket_->connect( tcp::endpoint( boost::asio::ip::address::from_string(srv), port ));

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

	boost::asio::write(*socket_, boost::asio::buffer(msg.c_str(), msg.size()) );

	if ( expectReply ) {
		boost::system::error_code error;
        boost::asio::streambuf receive_buffer;
        boost::asio::read(*socket_, receive_buffer, boost::asio::transfer_all(), error);
        if( error && error != boost::asio::error::eof ) {
			return "ERROR_RECV";
        }

        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
		return data;
	}
    return "";
}
