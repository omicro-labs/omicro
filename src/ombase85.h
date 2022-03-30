#ifndef _om_base85_h_
#define _om_base85_h_

#include <ascii85/ascii85.h>
#include "omicrodef.h"

void base85Encode( const unsigned char *inbuf, size_t inlen, sstr &outstr );
int  base85Decode( const char *inbuf, size_t inlen, unsigned char *outbuf, size_t outlen );
int  base85Decode( const sstr &instr, unsigned char *outbuf, size_t outlen );

#endif
