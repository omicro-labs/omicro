#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <omutil.h>
#include <omicrodef.h>
#include <omicrokey.h>
#include <omlog.h>

INIT_LOGGING

int main(int argc, char* argv[])
{
	sstr sk, pk;

	if ( argc == 2 && strcmp(argv[1], "node")==0 ) {
		OmicroNodeKey::createKeyPairSB3( sk, pk);
		printf("%s %s\n", s(sk), s(pk) );
	} else if  (argc ==2 && strcmp(argv[1], "user")==0 ) {
		OmicroUserKey::createKeyPairDL5( sk, pk);
		printf("%s %s\n", s(sk), s(pk) );
	} else {
		printf("Usage: %s node/user\n", argv[0] );
	}

    return 0;
}

