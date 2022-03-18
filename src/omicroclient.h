#ifndef _omicro_client_h_
#define _omicro_client_h_

#include <string>
#include "essential/utility/strutil.h"
#include "util/ikcp.h"
#include "client_lib/kcp_client_util.h"
#include "client_lib/kcp_client_wrap.hpp"
#include "omicrodef.h"


void omicro_client_event_callback(kcp_conv_t conv, asio_kcp::eEventType event_type, const sstr& msg, void* var);

class OmicroClient
{
  public:
  	OmicroClient( const char *host, int port, int retry=30);
  	~OmicroClient();

	sstr sendMessage( const sstr &msg, int waitMS );
	bool connectOK() const { return connectOK_; }
    sstr reply_;

  protected:
	asio_kcp::kcp_client_wrap client_;
	bool connectOK_;

};

#endif
