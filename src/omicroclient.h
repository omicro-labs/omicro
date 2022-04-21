#ifndef _omicro_client_h_
#define _omicro_client_h_

#include <string>

class OmicroTrxn;

class OmicroClient
{
  public:
  	OmicroClient( const char *host, int port );
  	~OmicroClient();

	std::string sendTrxn( OmicroTrxn &t, int waitSeconds=60 );
	std::string sendQuery( OmicroTrxn &t, int waitSeconds=60 );
	std::string sendMessage( char mtype, const std::string &msg, bool expectReply );
	std::string reqPublicKey( int waitSeconds);

	bool connectOK() const { return connectOK_; }

  protected:
	bool connectOK_;
	int  socket_;
	std::string srv_;
	int  port_;
};

#endif
