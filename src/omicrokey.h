#ifndef _omicro_nodekey_h_
#define _omicro_nodekey_h_

#include <string>

class OmicroNodeKey
{
  public:
  	OmicroNodeKey();
  	~OmicroNodeKey();

	static void createKeyPairSB3( std::string &secretKey, std::string &publicKey );

	static void encryptSB3( const std::string &msg, const std::string &publicKey, std::string &cipher,
	                     std::string &passwd, std::string &encMsg );
	static void decryptSB3( const std::string &encMsg, const std::string &secretKey, 
						 const std::string &cipher, std::string &plain );

	static void signSB3( const std::string &msg, const std::string &pubKey, 
					  std::string &cipher, std::string &signature );
	static bool verifySB3(const std::string &msg, const std::string &signature, 
					   const std::string &cipher, const std::string &secretKey);

	static void getHash( const std::string & msg, std::string &hash );
};

class OmicroUserKey
{
  public:
  	OmicroUserKey();
  	~OmicroUserKey();

	static void createKeyPairDL5( std::string &secretKey, std::string &publicKey );
	static void getUserId( const std::string &publicKey, std::string &userId );

	static void signDL5( const std::string &msg, const std::string &secretKey, std::string &snmsg );
	static bool verifyDL5(const std::string &sm, const std::string &pubKey );

};

#endif
