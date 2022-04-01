#include "omicrokey.h"
#include "ombase85.h"
#include "omaes.h"
#include "omutil.h"
#include "xxHash/xxhash.h"
EXTERN_LOGGING

OmicroKey::OmicroKey()
{
}

OmicroKey::~OmicroKey()
{
}


void OmicroKey::createKeyPair(sstr &secretKey, sstr &publicKey )
{
	uint8_t pk[CRYPTO_PUBLICKEYBYTES];
	uint8_t sk[CRYPTO_SECRETKEYBYTES];
	unsigned char entropy_input[48];

	for (int i=0; i<48; i++) {
		entropy_input[i] = i;
	}
	randombytes_init(entropy_input, NULL, 256);

	crypto_kem_keypair(pk, sk);
	base85Encode( sk, CRYPTO_SECRETKEYBYTES, secretKey );
	base85Encode( pk, CRYPTO_PUBLICKEYBYTES, publicKey );
}

void OmicroKey::encrypt( const sstr &msg, const sstr &publicKey, sstr &cipher, sstr &passwd, sstr &encMsg )
{
	uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
	uint8_t ss[CRYPTO_BYTES];
	uint8_t pk[CRYPTO_PUBLICKEYBYTES];

	base85Decode( publicKey, pk, CRYPTO_PUBLICKEYBYTES);
	crypto_kem_enc(ct, ss, pk);
	base85Encode( ct, CRYPTO_CIPHERTEXTBYTES, cipher );
	base85Encode( ss, CRYPTO_BYTES, passwd );
	aesEncrypt( msg, passwd, encMsg );
}

void OmicroKey::decrypt( const sstr &encMsg, const sstr &secretKey, const sstr &cipher, sstr &plain )
{
	uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
	uint8_t ss[CRYPTO_BYTES];
	uint8_t sk[CRYPTO_SECRETKEYBYTES];
	sstr passwd;

	base85Decode( secretKey, sk, CRYPTO_SECRETKEYBYTES);
	base85Decode( cipher, ct, CRYPTO_CIPHERTEXTBYTES);
	crypto_kem_dec(ss, ct, sk);
	base85Encode( ss, CRYPTO_BYTES, passwd );
	aesDecrypt( encMsg, passwd, plain );
}


void OmicroKey::sign( const sstr &msg, const sstr &pubKey, sstr &cipher, sstr &signature )
{
	XXH64_hash_t hash = XXH64(msg.c_str(), msg.size(), 0 );
	char buf[32];
	sprintf(buf, "%lu", hash);
	sstr hs(buf);
	//d("a22330 sign data msg=[%s] msglen=%lu", s(msg), msg.size() );
	//d("a22330 sign hash=%lu hash str=[%s]", hash, s(hs) );

	sstr passwd;
	encrypt( hs, pubKey, cipher, passwd, signature);
}

bool OmicroKey::verify(const sstr &msg, const sstr &signature, const sstr &cipher, const sstr &secretKey )
{
	XXH64_hash_t hash = XXH64(msg.c_str(), msg.size(), 0 );

	//d("a00288 verify msg=[%s] msglen=%d", s(msg), msg.size() );
	//d("a00288 ciper.len=%ld  seckeylen=%ld  signature.len=%ld\n", cipher.size(), secretKey.size(), signature.size() );
	
	sstr hashPlain;
	decrypt( signature, secretKey, cipher, hashPlain);
	char *ptr;
	unsigned long hashv = strtoul(hashPlain.c_str(), &ptr, 10);

	if ( hash == hashv ) {
		return true;
	} else {
		d("a22272838 msghash=%lu embeddedhashv=%lu  NEQ", hash, hashv );
		return false;
	}
}

