#ifndef _omicro_client_h_
#define _omicro_client_h_

#include <string>
#include "essential/utility/strutil.h"
#include "util/ikcp.h"
#include "client_lib/kcp_client_util.h"
#include "client_lib/kcp_client_wrap.hpp"


void omicro_client_event_callback(kcp_conv_t conv, asio_kcp::eEventType event_type, const std::string& msg, void* var);

class OmicroClient
{
  public:
  	OmicroClient( const char *host, int port, int retry=10);
  	~OmicroClient();

	std::string sendMessage( const std::string &msg, int waitMS );
	bool connectOK() const { return connectOK_; }
    std::string reply_;

  protected:
	asio_kcp::kcp_client_wrap client_;
	bool connectOK_;

};

#endif
