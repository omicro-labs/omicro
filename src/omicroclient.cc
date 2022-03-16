#include <iostream>
#include "omicroclient.h"

OmicroClient::OmicroClient( const char *srv, int port, int retry )
{
	connectOK_ = false;
    int rc = client_.connect(0, srv, port, retry );
    if ( rc < 0 ) {
		return;
    }
	std::cout << "a70231 OmicroClient connectOK_ srv=" << srv << " port=" << port << std::endl;
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		std::cout << "a72231 dtor of OmicroClient client_.stop()" << std::endl;
		client_.stop();
	}
}

sstr OmicroClient::sendMessage( const sstr &msg, int waitMilliSec )
{
	if ( ! connectOK_ ) {
		std::cout << "a52031 sendMessage return empty because connect is not OK" << std::endl;
		return "";
	}

	client_.set_event_callback(omicro_client_event_callback, (void*)this);

	client_.send_msg( msg );
	asio_kcp::millisecond_sleep(waitMilliSec);
	return reply_;
}

void omicro_client_event_callback(kcp_conv_t conv, asio_kcp::eEventType event_type, const sstr& msg, void* var)
{
	OmicroClient *obj = (OmicroClient*)var;

    // std::cout << "event_type: " << event_type << " msg: " << msg << std::endl;
    std::cout << "event_type str: " << asio_kcp::clientEventTypeStr(event_type) << std::endl;
    std::cout << "msg: " << msg << std::endl;
	if ( event_type == asio_kcp::eRcvMsg ) {
		obj->reply_ = msg;
	} 
}

