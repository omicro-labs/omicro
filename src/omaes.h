#ifndef _om_aes_h_
#define _om_aes_h_

#define AES256 1

//#include <tiny-AES-c/aes.hpp>
#include "omicrodef.h"

void aesEncrypt( const sstr &instr, const sstr &passwd, sstr &cipher );
void aesDecrypt( const sstr &instr, const sstr &passwd, sstr &plaintext );

#endif
