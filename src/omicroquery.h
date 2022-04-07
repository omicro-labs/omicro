#ifndef _omicro_query_h_
#define _omicro_query_h_

#include <string>

class OmicroQuery
{
  public:
  	OmicroQuery();
  	~OmicroQuery();

	void setTrxnId( const std::string &id);
	void setSender( const std::string &from);
	void setTimeStamp( const std::string &ts);
	void str( const std::string &qype, std::string &data);
	void strGetPublicKey( std::string &pk);

  protected:
  	std::string trnxId_;
  	std::string sender;
  	std::string timestamp;
};

#endif
