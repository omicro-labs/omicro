#ifndef _omicro_key_h_
#define _omicro_key_h_

#include "omicrodef.h"

class OmicroKey
{
  public:
  	OmicroKey();
  	~OmicroKey();

	int createKeyPair( const sstr &salt, sstr &secretKey, sstr &publicKey );
	sstr encrypt( const sstr &msg ); 
	sstr decrypt( const sstr &msg ); 

};

#endif
