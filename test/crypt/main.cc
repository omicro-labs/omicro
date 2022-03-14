// main.cc file
#include <stdio.h>
#include <assert.h>
#include "omicrokey.h"

int main( int argc, char *argv[] )
{
	OmicroKey k;
	sstr secretKey, publicKey;
	k.createKeyPair( 3, "hihimypassword", secretKey, publicKey );
	printf("secretKey=[%s]\n", secretKey.c_str() );
	printf("publicKey=[%s]\n", publicKey.c_str() );

	sstr msg = "do ntru keys";
	sstr encmsg = k.encrypt( 3, msg, secretKey );
	sstr msg2 = k.decrypt( 3, encmsg, publicKey );

	if ( msg2 == msg ) {
		printf("OK\n");
	} else {
		printf("error\n");
	}

	printf("hello\n");
	return 0;
}
