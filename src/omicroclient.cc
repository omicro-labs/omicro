#include <iostream>
#include "omicroclient.h"

OmicroClient::OmicroClient( const char *srv, int port, int retry )
{
	connectOK_ = false;
    int rc = client_.connect(0, srv, port, retry );
    if ( rc < 0 ) {
		return;
    }
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		client_.stop();
	}
}

std::string OmicroClient::sendMessage( const std::string &msg, int waitMilliSec )
{
	if ( ! connectOK_ ) {
		return "";
	}

	client_.set_event_callback(omicro_client_event_callback, (void*)this);

	client_.send_msg( msg );
	asio_kcp::millisecond_sleep(waitMilliSec);
	return reply_;
}

void omicro_client_event_callback(kcp_conv_t conv, asio_kcp::eEventType event_type, const std::string& msg, void* var)
{
	OmicroClient *obj = (OmicroClient*)var;

    std::cout << "event_type: " << event_type << " msg: " << msg << std::endl;
    std::cout << "event_type str: " << asio_kcp::clientEventTypeStr(event_type) << std::endl;
    std::cout << "msg: " << msg << std::endl;
	if ( event_type == asio_kcp::eRcvMsg ) {
		obj->reply_ = msg;
	} 
}
