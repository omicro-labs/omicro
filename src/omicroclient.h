#ifndef _omicro_client_h_
#define _omicro_client_h_

#include <string>
#include <cstdlib>
#include <cstring>
#include "omicrodef.h"


class OmicroTrxn;

class OmicroClient
{
  public:
  	OmicroClient( const char *host, int port );
  	~OmicroClient();

	sstr sendTrxn( OmicroTrxn &t, int waitSeconds=10 );
	sstr sendMessage( char mtype, const sstr &msg, bool expectReply );
	bool connectOK() const { return connectOK_; }

  protected:
	bool connectOK_;
	int  socket_;
	sstr srv_;
	int  port_;
};

#endif
