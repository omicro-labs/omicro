#ifndef _omicro_key_h_
#define _omicro_key_h_

#include "omicrodef.h"
#include "src/ntru.h"

class OmicroKey
{
  public:
  	OmicroKey();
  	~OmicroKey();

	// level: 1 moderate; 2 high;  3 highest
	int createKeyPair( int level, const sstr &salt, sstr &secretKey, sstr &publicKey );
	// secretKey: "3|34|3|-30|8|9|-2|...."
	// publicKey: "5|31|3|-30|2|-4|8|...."

	sstr encrypt( int level, const sstr &msg, const sstr &secretKey ); 
	sstr decrypt( int level, const sstr &encmsg, const sstr &publicKey ); 

	sstr sign( int level, const sstr &msg, const sstr &secretKey );
	bool verify( int level, const sstr &signedmsg, const sstr &publicKey );

};

#endif
