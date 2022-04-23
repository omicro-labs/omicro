#include <string>
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
	OmicroNodeKey::createKeyPairSB3( sk, pk);
	printf("%s %s\n", s(sk), s(pk) );
    return 0;
}

