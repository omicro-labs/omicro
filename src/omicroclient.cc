#include <iostream>
#include "omicroclient.h"
#include "omutil.h"
EXTERN_LOGGING

OmicroClient::OmicroClient( const char *srv, int port, int retry )
{
	connectOK_ = false;
    int rc = client_.connect(0, srv, port, retry );
    if ( rc < 0 ) {
		d("a002381 OmicroClient ctor connect to srv=[%s] port=%d retry=%d failed. connectOK_ is false rc=%d", srv, port, retry, rc );
		return;
    }
	d("a70231 OmicroClient connectOK_ srv=%s port=%d", srv, port);
	connectOK_ = true;
}

OmicroClient::~OmicroClient()
{
	if ( connectOK_ ) {
		d("a72231 dtor of OmicroClient client_.stop()");
		client_.stop();
	}
}

sstr OmicroClient::sendMessage( const sstr &msg, int waitMilliSec )
{
	if ( ! connectOK_ ) {
		d("a52031 sendMessage return empty because connect is not OK");
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
    d("event_type str: %s msg=[%s]", asio_kcp::clientEventTypeStr(event_type), msg.c_str() );
    // std::cout << "msg: " << msg << std::endl;
	if ( event_type == asio_kcp::eRcvMsg ) {
		obj->reply_ = msg;
	} 
}

