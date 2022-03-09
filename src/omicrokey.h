#ifndef _omicro_key_h_
#define _omicro_key_h_

#include <string>

class OmicroKey
{
  public:
  	OmicroKey();
  	~OmicroKey();

	int createKeyPair( const std::string &salt, std::string &secretKey, std::string &publicKey );
	std::string encrypt( const std::string &msg ); 
	std::string decrypt( const std::string &msg ); 

};

#endif
