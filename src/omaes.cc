#include <string.h>
#include <stdlib.h>
#include "omicrodef.h"
#include "omaes.h"
#include "ombase85.h"
#include "xxHash/xxhash.h"


#if 0
#include <rpc/des_crypt.h>
// instr has non-nulls in it
void aesEncrypt( const sstr &instr, const sstr &passwd, sstr &cipher )
{
	int klen = passwd.size();
	if ( klen < 4 ) {
		return;
	}

	//printf("a22334 aesEncrypt instr=[%s] size=%ld\n", instr.c_str(), instr.size() );
	char key[8+1];

	XXH64_hash_t hash;
	/**
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	sprintf(key, "%08lu", hash);
	**/
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000 ;
	sprintf(key, "%c%c%c%05lu", passwd[0], passwd[1], passwd[2], hash);

	//printf("a93938 aesEncrypt key=[%s]\n", key );
	des_setparity(key);

	int inlen = instr.size();
	int remain = inlen % 8;    // e.g.3
	//int totlen = inlen + 8-remain +1;
	int totlen = inlen + 8-remain;
	char data[totlen];
	memcpy(data, instr.c_str(), inlen);
	memset(data+inlen, 0, remain);

	char ivec[9];
	sprintf(ivec,"87654321");

	int n = inlen;
    while (n % 8 && n < totlen) {
        data[n++] = '\0';
    }

	//printf("a2223902 cbc_crypt DES_ENCRYPT data=[%s] n=%d \n", data, n );
	int res = cbc_crypt(key, data, n, DES_ENCRYPT | DES_SW, ivec);
	//int res = ecb_crypt(key, data, n, DES_ENCRYPT | DES_SW );
	//if (DES_FAILED(res) || strcmp(data, "") == 0) {
	if (DES_FAILED(res) ) {
		/***
		printf("a2223902 cbc_crypt DES_ENCRYPT failed. inlen=%d res=%d data=[%s]\n", inlen, res, data );
		for ( int j=0; j < n; ++j ) {
			if ( '\0' == data[j] ) {
				printf("j=%d NULL\n", j );
			} else {
				printf("j=%d [%c]\n", j, data[j] );
			}
		}
		if ( DESERR_NONE == res ) {
			printf("DESERR_NONE OK\n");
		}
		***/
		return;
	}

	base85Encode( (unsigned char*)data, n, cipher );
	/**
	printf("a3192  aesencrypt cipher=[%s]\n", cipher.c_str() );
	printf("a32220 aesencrypt size=%lu strlen=%lu inlen=%d\n", cipher.size(), strlen(cipher.c_str()), inlen );
	**/
}

// instr is base85 encoded ciphertext
void aesDecrypt( const sstr &instr, const sstr &passwd, sstr &plaintext )
{
	int klen = passwd.size();
	if ( klen < 4 ) {
		return;
	}

	size_t inlen = instr.size();
	char data[inlen];
	size_t plainlen = base85Decode( instr, (unsigned char*)data, inlen);

	char key[8+1];
	XXH64_hash_t hash;
	/**
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	sprintf(key, "%08lu", hash);
	**/
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000 ;
	sprintf(key, "%c%c%c%05lu", passwd[0], passwd[1], passwd[2], hash);
	//printf("a93939 aesDecrypt key=[%s]\n", key );
	des_setparity(key);

	char ivec[9];
	sprintf(ivec,"87654321");

	int res = cbc_crypt(key, data, plainlen, DES_DECRYPT | DES_SW, ivec);
	//int res = ecb_crypt(key, data, plainlen, DES_DECRYPT | DES_SW );
	//if (DES_FAILED(res) || strcmp(data, "") == 0) {
	if (DES_FAILED(res) ) {
		return;
	}

	size_t dlen = strlen(data);
	size_t flen = ( dlen < plainlen ) ? dlen : plainlen;
	plaintext = sstr((char*)data, flen);
	/***
	printf("a2220 aesDecrypt plainlen=%lu size=%lu strlen=%lu\n", plainlen, plaintext.size(), strlen(plaintext.c_str()) );
	printf("inlen=%lu  plainlen=%lu \n", inlen, plainlen );
	printf("a1192  aesDecrypt data.strlen=[%lu]\n", strlen(data) );
	printf("a1192  aesDecrypt flen=[%lu]\n", flen );
	printf("a1192  aesDecrypt plain=[%s]\n", plaintext.c_str());
	***/
}
#endif


#include <AES_192_CBC.h>
// instr has non-nulls in it
void aesEncrypt( const sstr &instr, const sstr &passwd, sstr &cipher )
{
	int klen = passwd.size();
	if ( klen < 16 ) {
		return;
	}

	//printf("a22334 aesEncrypt instr=[%s] size=%ld\n", instr.c_str(), instr.size() );
    AES_CTX ctx;

	//char key[8+1];
	uint8_t key[AES_KEY_SIZE+1]; // 24
    uint8_t iv[AES_BLOCK_SIZE]; // 16


	XXH64_hash_t hash8;
	/**
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	sprintf(key, "%08lu", hash);
	**/
	hash8 = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	//sprintf(key, "%c%c%c%c%c%c%c%c%c%08lu", passwd[0], passwd[1], passwd[2], passwd[3], passwd[4], passwd[5], passwd[6], passwd[7], hash8);

    //key[AES_KEY_SIZE] = '\0';
    strncpy( (char*)key, passwd.c_str(), 16 );
    sprintf( (char*)key+16, "%08lu", hash8);

    memcpy(iv, "This_is_an_IV123", AES_BLOCK_SIZE); // exactly 16 bytes
    AES_EncryptInit(&ctx, key, iv);


	//printf("a93938 aesEncrypt key=[%s]\n", key );
	//des_setparity(key);

	int inlen = instr.size();

	// int remain = inlen % 8;    // e.g.3
	int remain = inlen % AES_BLOCK_SIZE;    // e.g.3
	//int totlen = inlen + 8-remain;
	unsigned int totlen = inlen + AES_BLOCK_SIZE-remain;
	uint8_t data[totlen];
	memcpy(data, instr.c_str(), inlen);
	memset(data+inlen, 0, remain);

	//char ivec[9];
	//sprintf(ivec,"87654321");

	unsigned int n = inlen;
    while (n % AES_BLOCK_SIZE && n < totlen) {
        data[n++] = '\0';
    }

    for (unsigned int s = 0; s < totlen; s += AES_BLOCK_SIZE) {
        AES_Encrypt(&ctx, data + s, data + s);
    }


	base85Encode( (unsigned char*)data, n, cipher );
    AES_CTX_Free(&ctx);

	/**
	printf("a3192  aesencrypt cipher=[%s]\n", cipher.c_str() );
	printf("a32220 aesencrypt size=%lu strlen=%lu inlen=%d\n", cipher.size(), strlen(cipher.c_str()), inlen );
	**/
}

// instr is base85 encoded ciphertext
void aesDecrypt( const sstr &instr, const sstr &passwd, sstr &plaintext )
{
	int klen = passwd.size();
	if ( klen < 16 ) {
		return;
	}

    AES_CTX ctx;
	uint8_t key[AES_KEY_SIZE+1]; // 24
    uint8_t iv[AES_BLOCK_SIZE]; // 16


	// char key[8+1];
	XXH64_hash_t hash8;
	/**
	hash = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	sprintf(key, "%08lu", hash);
	**/

	hash8 = XXH64( passwd.c_str(), passwd.size(), 8888 ) % 100000000 ;
	//sprintf(key, "%c%c%c%05lu", passwd[0], passwd[1], passwd[2], hash);
    strncpy( (char*)key, passwd.c_str(), 16 );
    sprintf( (char*)key+16, "%08lu", hash8);

    memcpy(iv, "This_is_an_IV123", AES_BLOCK_SIZE); // exactly 16 bytes
    AES_DecryptInit(&ctx, key, iv);

	size_t  inlen = instr.size();
	//char    data[inlen];
	uint8_t    data[inlen];
	size_t  plainlen = base85Decode( instr, (unsigned char*)data, inlen);
    // base85 string to original encoded data 

    for (unsigned int s = 0; s < plainlen; s += AES_BLOCK_SIZE) {
        AES_Decrypt(&ctx, data + s, data + s);
    }

    AES_CTX_Free(&ctx);


	//printf("a93939 aesDecrypt key=[%s]\n", key );
	// des_setparity(key);

	//char ivec[9];
	//sprintf(ivec,"87654321");

    /***
	int res = cbc_crypt(key, data, plainlen, DES_DECRYPT | DES_SW, ivec);
	//int res = ecb_crypt(key, data, plainlen, DES_DECRYPT | DES_SW );
	//if (DES_FAILED(res) || strcmp(data, "") == 0) {
	if (DES_FAILED(res) ) {
		return;
	}
    ***/

	size_t dlen = strlen((char*)data);
	size_t flen = ( dlen < plainlen ) ? dlen : plainlen;
	plaintext = sstr((char*)data, flen);
	/***
	printf("a2220 aesDecrypt plainlen=%lu size=%lu strlen=%lu\n", plainlen, plaintext.size(), strlen(plaintext.c_str()) );
	printf("inlen=%lu  plainlen=%lu \n", inlen, plainlen );
	printf("a1192  aesDecrypt data.strlen=[%lu]\n", strlen(data) );
	printf("a1192  aesDecrypt flen=[%lu]\n", flen );
	printf("a1192  aesDecrypt plain=[%s]\n", plaintext.c_str());
	***/
}
