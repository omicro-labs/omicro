/*
 * Copyright (C) Omicro Authors
 *
 * Omicro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Omicro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the LICENSE file. If not, see <http://www.gnu.org/licenses/>.
 */
#include <string.h>
#include "omicrokey.h"
#include "ombase85.h"
#include "omaes.h"
#include "omutil.h"
#include "xxHash/xxhash.h"
#include "saber.h"
#include "dilith.h"
#include "omlog.h"

EXTERN_LOGGING

//#define OM_DEBUG_KEY 1

/************************** node key *************************
*
*************************************************************/
OmicroNodeKey::OmicroNodeKey()
{
}

OmicroNodeKey::~OmicroNodeKey()
{
}


void OmicroNodeKey::createKeyPairSB3(sstr &secretKey, sstr &publicKey )
{
	uint8_t pk[CRYPTO_PUBLICKEYBYTES];
	uint8_t sk[CRYPTO_SECRETKEYBYTES];
	crypto_kem_keypair(pk, sk);
	base85Encode( sk, CRYPTO_SECRETKEYBYTES, secretKey );
	base85Encode( pk, CRYPTO_PUBLICKEYBYTES, publicKey );
}

void OmicroNodeKey::encryptSB3( const sstr &msg, const sstr &publicKey, sstr &cipher, sstr &passwd, sstr &encMsg )
{
	uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
	uint8_t ss[CRYPTO_BYTES];
	uint8_t pk[CRYPTO_PUBLICKEYBYTES];

	base85Decode( publicKey, pk, CRYPTO_PUBLICKEYBYTES);

	crypto_kem_enc(ct, ss, pk);

	base85Encode( ct, CRYPTO_CIPHERTEXTBYTES, cipher );
	#ifdef OM_DEBUG_KEY
		uint8_t ct2[CRYPTO_CIPHERTEXTBYTES];
		base85Decode( cipher, ct2, CRYPTO_CIPHERTEXTBYTES);
		for ( int j=0; j < CRYPTO_CIPHERTEXTBYTES; ++j ) {
			if ( ct2[j] != ct[j] ) {
				i("k222021 cipher problem");	
				break;
			}
		}
	#endif

	base85Encode( ss, CRYPTO_BYTES, passwd );
	#ifdef OM_DEBUG_KEY
		uint8_t ss2[CRYPTO_BYTES];
		base85Decode( passwd, ss2, CRYPTO_BYTES);
		for ( int j=0; j < CRYPTO_BYTES; ++j ) {
			if ( ss2[j] != ss[j] ) {
				i("k222024 ss problem");	
				break;
			}
		}
	#endif

	aesEncrypt( msg, passwd, encMsg );
	#ifdef OM_DEBUG_KEY
		sstr plain;
		aesDecrypt( encMsg, passwd, plain );
		if ( plain != msg ) {
			i("k224025 aesEncrypt/aesDecrypt problem");	
			i("k224025 encMsg=[%s] size=%lu strlen=%lu", s(encMsg), encMsg.size(), strlen(encMsg.c_str()) );
			i("k224025 msg  =[%s] msg.len=%d", s(msg), msg.size() );
			i("k224025 plain=[%s] pln.len=%d", s(plain), plain.size() );
			i("k224025 passwd=[%s] passwd.size=%lu strlen=%lu", s(passwd), passwd.size(), strlen(passwd.c_str()) );
		}
	#endif

}

void OmicroNodeKey::decryptSB3( const sstr &encMsg, const sstr &secretKey, const sstr &cipher, sstr &plain )
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

void OmicroNodeKey::signSB3( const sstr &msg, const sstr &pubKey, sstr &cipher, sstr &signature )
{
	sstr hashStr;
	getHash( msg, hashStr );

	sstr passwd;
	encryptSB3( hashStr, pubKey, cipher, passwd, signature);

	/***
	d("a22330 signSB3 sign data msg=[%s] msglen=%lu", s(msg), msg.size() );
	d("a22330 signSB3 hashstr=[%s]", s(hashStr) );
	d("a22330 signSB3 pubKey=[%s]", s(pubKey) );
	d("a22330 signSB3 cipher=[%s]", s(cipher) );
	d("a22330 signSB3 signature=[%s]", s(signature) );
	***/
}

bool OmicroNodeKey::verifySB3(const sstr &msg, const sstr &signature, const sstr &cipher, const sstr &secretKey )
{
	sstr hashStr;
	getHash( msg, hashStr );
	/***
	d("a00288 verifySB3 verify msg=[%s]", s(msg) );
	d("a00288 msglen=%d", msg.size() );
	d("a00288 ciper.len=%ld  seckeylen=%ld  signature.len=%ld\n", cipher.size(), secretKey.size(), signature.size() );
	***/
	
	sstr hashPlain;
	decryptSB3( signature, secretKey, cipher, hashPlain);
	//d("a122291 decryptSB3 hashPlain=[%s]", hashPlain.c_str() );

	if ( hashStr == hashPlain ) {
		/**
		d("a44291 verifySB3 OK signature=[%s]", s(signature) ); 
		d("a44291 verifySB3 OK cipher=[%s]", s(cipher) ); 
		d("a44291 verifySB3 OK secretKey=[%s]", s(secretKey) ); 
		**/
		return true;
	} else {
		i("E330126 error recomputed hashStr=[%s] embeddedhashstr=[%s]  NEQ", s(hashStr), s(hashPlain) );
		return false;
	}
}

void OmicroNodeKey::getHash( const std::string &msg, std::string &hashStr )
{
	char buf[48];
	XXH64_hash_t hashv = XXH64(msg.c_str(), msg.size(), 0 );
	sprintf(buf, "z%lx", hashv);
	hashStr = buf;
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


void OmicroUserKey::createKeyPairDL5(sstr &secretKey, sstr &publicKey )
{
	uint8_t pk[CRYPTO_PUBLICKEYBYTES_DL];
	uint8_t sk[CRYPTO_SECRETKEYBYTES_DL];
	crypto_sign_keypair(pk, sk);
	base85Encode( sk, CRYPTO_SECRETKEYBYTES_DL, secretKey );
	base85Encode( pk, CRYPTO_PUBLICKEYBYTES_DL, publicKey );
}

void OmicroUserKey::signDL5( const sstr &msg, const sstr &secretKey, sstr &snmsg )
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

bool OmicroUserKey::verifyDL5(const sstr &snmsg, const sstr &pubKey )
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

