#ifndef _omicro_nodekey_h_
#define _omicro_nodekey_h_

#include "omicrodef.h"
#include "saber.h"
#include "dilith.h"

class OmicroNodeKey
{
  public:
  	OmicroNodeKey();
  	~OmicroNodeKey();

	static void createKeyPair( sstr &secretKey, sstr &publicKey );

	static void encrypt( const sstr &msg, const sstr &publicKey, sstr &cipher, sstr &passwd, sstr &encMsg );
	static void decrypt( const sstr &encMsg, const sstr &secretKey, const sstr &cipher, sstr &plain );

	static void sign( const sstr &msg, const sstr &pubKey, sstr &cipher, sstr &signature );
	static bool verify(const sstr &msg, const sstr &signature, const sstr &cipher, const sstr &secretKey);

};

class OmicroUserKey
{
  public:
  	OmicroUserKey();
  	~OmicroUserKey();

	static void createKeyPair( sstr &secretKey, sstr &publicKey );

	static void sign( const sstr &msg, const sstr &secretKey, sstr &snmsg );
	static bool verify(const sstr &sm, const sstr &pubKey );

};

#endif
