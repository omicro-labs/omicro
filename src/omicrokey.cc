#include "omicrokey.h"
#include "ombase85.h"
#include "omaes.h"
#include "omutil.h"
#include "xxHash/xxhash.h"
EXTERN_LOGGING

/************************** node key *************************
*
*************************************************************/
OmicroNodeKey::OmicroNodeKey()
{
}

OmicroNodeKey::~OmicroNodeKey()
{
}


void OmicroNodeKey::createKeyPair(sstr &secretKey, sstr &publicKey )
{
	uint8_t pk[CRYPTO_PUBLICKEYBYTES];
	uint8_t sk[CRYPTO_SECRETKEYBYTES];
	/**
	unsigned char entropy_input[48];
	for (int i=0; i<48; i++) {
		entropy_input[i] = i;
	}
	randombytes_init(entropy_input, NULL, 256);
	**/
	crypto_kem_keypair(pk, sk);
	base85Encode( sk, CRYPTO_SECRETKEYBYTES, secretKey );
	base85Encode( pk, CRYPTO_PUBLICKEYBYTES, publicKey );
}

void OmicroNodeKey::encrypt( const sstr &msg, const sstr &publicKey, sstr &cipher, sstr &passwd, sstr &encMsg )
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

void OmicroNodeKey::decrypt( const sstr &encMsg, const sstr &secretKey, const sstr &cipher, sstr &plain )
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


void OmicroNodeKey::sign( const sstr &msg, const sstr &pubKey, sstr &cipher, sstr &signature )
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

bool OmicroNodeKey::verify(const sstr &msg, const sstr &signature, const sstr &cipher, const sstr &secretKey )
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

/******************** user key *****************************************
*
************************************************************************/
OmicroUserKey::OmicroUserKey()
{
}

OmicroUserKey::~OmicroUserKey()
{
}


void OmicroUserKey::createKeyPair(sstr &secretKey, sstr &publicKey )
{
	uint8_t pk[CRYPTO_PUBLICKEYBYTES_DL];
	uint8_t sk[CRYPTO_SECRETKEYBYTES_DL];
	crypto_sign_keypair(pk, sk);
	base85Encode( sk, CRYPTO_SECRETKEYBYTES_DL, secretKey );
	base85Encode( pk, CRYPTO_PUBLICKEYBYTES_DL, publicKey );
}

void OmicroUserKey::sign( const sstr &msg, const sstr &secretKey, sstr &snmsg )
{
	int MLEN = msg.size();
	uint8_t m[MLEN + CRYPTO_BYTES_DL];
	memcpy(m, msg.c_str(), MLEN);

	uint8_t sm[MLEN + CRYPTO_BYTES_DL];
	size_t smlen;

	uint8_t sk[CRYPTO_SECRETKEYBYTES_DL];
	base85Decode( secretKey, sk, CRYPTO_SECRETKEYBYTES_DL);
	
	crypto_sign(sm, &smlen, m, MLEN, sk);

	base85Encode( sm, smlen, snmsg );
	// printf("a22200 smlen=%ld MLEN=%d MLEN + CRYPTO_BYTES_DL=%ld\n", smlen, MLEN, MLEN + CRYPTO_BYTES_DL );
}

bool OmicroUserKey::verify(const sstr &snmsg, const sstr &pubKey )
{
	uint8_t pk[CRYPTO_PUBLICKEYBYTES_DL];
	base85Decode( pubKey, pk, CRYPTO_PUBLICKEYBYTES_DL);

	size_t MLEN = snmsg.size();
	uint8_t sm[MLEN + CRYPTO_BYTES_DL];
	int smlen = base85Decode( snmsg, sm, MLEN + CRYPTO_BYTES_DL );

	uint8_t m2[MLEN + CRYPTO_BYTES_DL];
	size_t mlen;

	int ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);
	if ( ret ) {
		d("a20028 OmicroUserKey::verify ret=%d false\n", ret );
		//d("a20028 snmsg=[%s]", s(snmsg) );
		//d("a20028 userpubKey=[%s]", s(pubKey) );
		//d("a20028 smlen=[%d]", smlen );
		return false;
	} else {
		return true;
	}
}

void OmicroUserKey::getUserId( const sstr &publicKey, sstr &userId )
{
	// get 24 byte ID from publicKey 
	int len = publicKey.size();
	if ( len < 500 ) {
		return;
	}
	userId = publicKey.substr(0, 6) + publicKey.substr(18, 6) 
	         + publicKey.substr( len - 256, 6) + publicKey.substr( len - 54, 6); 

}

