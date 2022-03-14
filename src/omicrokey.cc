#include "omicrokey.h"

OmicroKey::OmicroKey()
{
}

OmicroKey::~OmicroKey()
{
}


int OmicroKey::createKeyPair(int level, const sstr &salt, sstr &secretKey, sstr &publicKey )
{
	return 0;
}

sstr OmicroKey::encrypt( int level, const sstr &msg,  const sstr &secretKey )
{
	return "";
}

sstr OmicroKey::decrypt( int level, const sstr &encmsg, const sstr &publicKey )
{
	return "";
}


sstr OmicroKey::sign( int level, const sstr &msg, const sstr &secretKey )
{
	return "";
}

bool OmicroKey::verify(int level, const sstr &signedmsg, const sstr &publicKey )
{
	return true;
}


// convert NtruEncPubKey to str
void OmicroKey::pubKeyToString( const NtruEncPubKey &pkey, sstr &str )
{
}

// convert str to NtruEncPubKey
void stringToPubKey( const sstr &str,  NtruEncPubKey &pkey )
{
}


// convert NtruEncPrivKey to str
void privKeyToString( const NtruEncPrivKey &skey, sstr &str )
{
}

// convert str to NtruEncPrivKey 
void stringToPrivKey( const sstr &str, NtruEncPrivKey &skey )
{
}


