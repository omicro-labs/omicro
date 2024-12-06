// main.cc file
#include <stdio.h>
#include <assert.h>
#include "omicrokey.h"


using sstr = std::string;

int main( int argc, char *argv[] )
{
	OmicroNodeKey k;
	sstr secretKey, publicKey;
	k.createKeyPairSB3( secretKey, publicKey );
	printf("secretKey=[%s]\n", secretKey.c_str() );
	printf("publicKey=[%s]\n", publicKey.c_str() );

	sstr msg = "do ntru keys";
    sstr cipher, passwd, encMsg;
	k.encryptSB3( msg, publicKey, cipher, passwd, encMsg );

	printf("cipher=[%s]\n", cipher.c_str() );
	printf("passwd=[%s]\n", passwd.c_str() );
	printf("encMsg=[%s]\n", encMsg.c_str() );


	sstr plain;
    k.decryptSB3( encMsg, secretKey, cipher, plain );

	if ( plain == msg ) {
		printf("enc dec OK\n");
	} else {
		printf("error\n");
	}

	printf("done\n");
	return 0;
}
