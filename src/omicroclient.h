#ifndef _omicro_client_h_
#define _omicro_client_h_

#include <string>
#include <mutex>

class OmicroTrxn;

class OmicroClient
{
  public:
  	OmicroClient( const char *host, int port );
  	~OmicroClient();

	std::string sendTrxn( OmicroTrxn &t  );
	std::string sendQuery( OmicroTrxn &t );
	std::string sendMessage( char mtype, const std::string &msg, bool expectReply );
	std::string reqPublicKey( int waitSeconds);

	bool connectOK() const { return connectOK_; }

  protected:
	bool connectOK_;
	int  socket_;
	std::string srv_;
	int  port_;
	std::mutex  mutex_;
};

#endif
