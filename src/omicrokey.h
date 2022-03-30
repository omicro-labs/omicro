#ifndef _omicro_key_h_
#define _omicro_key_h_

#include "omicrodef.h"
#include "saber.h"

class OmicroKey
{
  public:
  	OmicroKey();
  	~OmicroKey();

	// level: 1 moderate; 2 high;  3 highest
	static void createKeyPair( sstr &secretKey, sstr &publicKey );
	// secretKey: "3|34|3|-30|8|9|-2|...."
	// publicKey: "5|31|3|-30|2|-4|8|...."

	static void encrypt( const sstr &msg, const sstr &publicKey, sstr &cipher, sstr &passwd, sstr &encMsg );
	static void decrypt( const sstr &encMsg, const sstr &secretKey, const sstr &cipher, sstr &plain );

	static void sign( const sstr &msg, const sstr &pubKey, sstr &cipher, sstr &signature );
	static bool verify(const sstr &msg, const sstr &signature, const sstr &cipher, const sstr &secretKey);

};

#endif
