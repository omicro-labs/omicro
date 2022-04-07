#include <string.h>
#include "omicrodef.h"
#include "omaes.h"
#include "ombase85.h"
#include "xxHash/xxhash.h"
#include <rpc/des_crypt.h>

// instr has non-nulls in it
void aesEncrypt( const sstr &instr, const sstr &passwd, sstr &cipher )
{
	int klen = passwd.size();
	if ( klen < 8 ) {
		return;
	}

	XXH64_hash_t hash;
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	char key[klen+1];
	sprintf(key, "%08ld", hash);
	des_setparity(key);

	int inlen = instr.size();
	int remain = inlen % 8;
	int totlen = inlen + remain + 8;
	char data[totlen];
	memcpy(data, instr.c_str(), inlen);

	char ivec[9];
	sprintf(ivec,"87654321");

	int n = inlen;
    while (n % 8 && n < totlen) {
        data[n++] = '\0';
    }

	int res = cbc_crypt(key, data, n, DES_ENCRYPT | DES_SW, ivec);
	if (DES_FAILED(res) || strcmp(data, "") == 0) {
		return;
	}

	base85Encode( (unsigned char*)data, n, cipher );
}

// instr is base85 encoded ciphertext
void aesDecrypt( const sstr &instr, const sstr &passwd, sstr &plaintext )
{
	int klen = passwd.size();
	if ( klen < 8 ) {
		return;
	}

	int inlen = instr.size();
	char data[inlen];
	int plainlen = base85Decode( instr, (unsigned char*)data, inlen);

	XXH64_hash_t hash;
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;

	char key[klen+1];
	sprintf(key, "%08ld", hash);
	des_setparity(key);

	char ivec[9];
	sprintf(ivec,"87654321");

	int res = cbc_crypt(key, data, plainlen, DES_DECRYPT | DES_SW, ivec);
	if (DES_FAILED(res) || strcmp(data, "") == 0) {
		return;
	}

	plaintext = sstr((char*)data, plainlen);
}

