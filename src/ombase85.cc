#include <ascii85/ascii85.h>
#include "ombase85.h"

void base85Encode( const unsigned char *inbuf, size_t inlen, sstr &outstr )
{
	int olen = ascii85_get_max_encoded_length(inlen);
    if ( olen < 0 ) {
		return;
    }

	char  obuf[olen];
	olen = encode_ascii85((uint8_t *)inbuf, inlen, (uint8_t*)obuf, olen);
	outstr = sstr(obuf, olen);
}

int base85Decode( const sstr &instr, unsigned char *outbuf, size_t outlen )
{
	int rc = base85Decode( instr.c_str(), instr.size(), outbuf, outlen );
	if ( rc < 0 ) {
		return rc;
	}
	return rc;
}

int base85Decode( const char *inbuf, size_t inlen, unsigned char *outbuf, size_t outlen )
{
	int olen = ascii85_get_max_decoded_length(inlen);
	if ( olen < 0 ) {
		return -1;
	}

	int ilen = decode_ascii85((uint8_t *)inbuf, inlen, (uint8_t*)outbuf, olen);
	if ( ilen < 0 ) {
		return -2;
	}

	return ilen;
}

