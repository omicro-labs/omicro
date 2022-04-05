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
#include <random>
#include <malloc.h>
#include <sys/time.h>
#include <string.h>
#include "omicrotrxn.h"
#include "omutil.h"
#include "omstrsplit.h"
#include "omicrokey.h"
EXTERN_LOGGING

OmicroTrxn::OmicroTrxn()
{
	hdr_ = "TT"; // plaintext, trxn
}

OmicroTrxn::OmicroTrxn( const char *str )
{
	OmStrSplit sp(str, '|');
	hdr_ = sp[0];
	id_ = sp[1];
	beacon_ = sp[2];
	srvport_ = sp[3];
	sender_ = sp[4];
	receiver_ = sp[5];
	amount_ = sp[6];
	timestamp_ = sp[7];
	trxntype_ = sp[8];
	assettype_ = sp[9];
	//vote_ = sp[10];

	pad1_ = sp[10];
	pad2_ = sp[11];
	pad3_ = sp[12];
	pad4_ = sp[13];
	pad5_ = sp[14];
	pad6_ = sp[15];
	pad7_ = sp[16];
	pad8_ = sp[17];
	pad9_ = sp[18];
	pad10_ = sp[19];

	//d("a33098 trxn str ctor pad9=[%s] pad10=[%s]", s(pad9_), s(pad10_) );

	// node PKI
	cipher_ = sp[20];
	signature_ = sp[21];

	// user account PKI
	userPubkey_ = sp[22];
	userSignature_ = sp[23];

	vote_ = sp[24];
}

OmicroTrxn::~OmicroTrxn()
{
}

void OmicroTrxn::setInitTrxn()
{
	hdr_[TRXN_HEADER_START] = 'I'; 
}

void OmicroTrxn::setNotInitTrxn()
{
	hdr_[TRXN_HEADER_START] = 'N'; 
}

bool OmicroTrxn::isInitTrxn()
{
	if ( 'I' == hdr_[TRXN_HEADER_START] ) {
		return true;
	} else {
		return false;
	}
}

void OmicroTrxn::setID()
{
    char s[16];
    std::uniform_int_distribution<uint64_t> distribution;
    std::mt19937_64   engine(std::random_device{}());
    uint64_t r1 = distribution(engine);
    sprintf(s, "%lx", r1 );
    id_ = s;
}

//2nd byte
void  OmicroTrxn::setXit( Byte xit)
{
	hdr_[TRXN_HEADER_START+1] = xit;
}

Byte OmicroTrxn::getXit()
{
	return hdr_[TRXN_HEADER_START+1];
}

void OmicroTrxn::setBeacon()
{
	char s[7+1];
    ulong pm = ipow(10, 7);
    sprintf(s, "%*d", 7, int(time(NULL)%pm) );
	beacon_ = s;
}


double OmicroTrxn::getAmountDouble()
{
	double f = atof(amount_.c_str());
	return f;
}

void OmicroTrxn::setNowTimeStamp()
{
	char tb[32];
    struct timeval now;
    gettimeofday( &now, NULL );
    ulong tot = now.tv_sec*1000000 + now.tv_usec;
    sprintf(tb, "%lu", tot );
	timestamp_ = tb;
}

ulong OmicroTrxn::getTimeStampUS()
{
	return atol(timestamp_.c_str());
	// microseconds since epoch
}

int OmicroTrxn::getVoteInt()
{
	int num = atoi(vote_.c_str());
	return num;
}

void OmicroTrxn::setVoteInt( int votes )
{
	char v[16];
	sprintf(v, "%d", votes );
	vote_ = v;
}

void OmicroTrxn::addVote(int vote)
{
	int v = getVoteInt();
	v += vote;
	setVoteInt( v );
}

void OmicroTrxn::minusVote(int vote)
{
	int v = getVoteInt();
	v -= vote;
	if ( v < 0 ) v = 0;
	setVoteInt( v );
}


void OmicroTrxn::makeNodeSignature( const sstr &nodePubKey)
{
	sstr data;
	getTrxnData( data );
	OmicroNodeKey::sign( data, nodePubKey, cipher_, signature_ );
	/**
	// debug
	d("a222128 makeNodeSignature trxndata=[%s]", s(data) );
	d("a222128 makeNodeSignature cipher=[%s]", s(cipher_) );
	d("a222128 makeNodeSignature signature=[%s]", s(signature_) );
	d("done makeNodeSignature\n");
	**/
}

void OmicroTrxn::allstr( sstr &alldata )
{
	sstr data;
	getTrxnData( data );
	alldata = data + "|" + cipher_ + "|" + signature_ + "|" 
	          + userPubkey_ + "|" + userSignature_ + "|" + vote_;
}

void OmicroTrxn::getTrxnID( sstr &id )
{
	id = timestamp_ + ":" + sender_;
}

bool OmicroTrxn::validateTrxn( const sstr &secretKey )
{
	ulong trxnTime = getTimeStampUS();
	unsigned long nowt = getNowTimeUS();
	if ( nowt - trxnTime > 60000000 ) {
		// lag of 60 seconds
		i("a303376 warn validateTrxn() nowt=%ld trxnTime=%ld more than 60 seconds", nowt, trxnTime);
		return false;
	}

	// signature verification
	sstr data;
	getTrxnData( data );
	bool rc = OmicroNodeKey::verify(data, signature_, cipher_, secretKey);
	if ( ! rc ) {
		d("a34408 nodekey verify false");
		return false;
	}
	/**
	d("a22208 trxndata=[%s] len=%d  rc=%d", s(data), data.size(), rc );
	d("a22208 secretKey=[%s] len=%d", s(secretKey), secretKey.size() );
	d("a22208 cipher=[%s] len=%d", s(cipher), cipher.size() );
	d("a22208 signature=[%s] len=%d", s(signature), signature.size() );
	**/

	rc = OmicroUserKey::verify(userSignature_, userPubkey_ );
	if ( ! rc ) {
		d("a34438 userkey verify false");
		return false;
	}
	/***
	d("a327639 OmicroUserKey::verify userSignature_=[%s]", s(userSignature_));
	d("a327639 OmicroUserKey::verify userPubkey_=[%s] rc=%d", s(userPubkey_), rc);
	***/

	if ( trxntype_ == OM_PAYMENT ) {
		double amt = getAmountDouble();
		if ( amt <= 0.0001 ) {
			d("a31538 amt=%.6f too small", amt );
			return false;
		}
	}

	return true;
}

void OmicroTrxn::getTrxnData( sstr &data )
{
	data = hdr_ +
	      + "|" + id_ 
	      + "|" + beacon_ 
	      + "|" + srvport_ 
	      + "|" + sender_ 
	      + "|" + receiver_ 
	      + "|" + amount_ 
	      + "|" + timestamp_ 
	      + "|" + trxntype_ 
	      + "|" + assettype_ 
	      + "|" + pad1_ 
	      + "|" + pad2_ 
	      + "|" + pad3_ 
	      + "|" + pad4_ 
	      + "|" + pad5_ 
	      + "|" + pad6_ 
	      + "|" + pad7_ 
	      + "|" + pad8_ 
	      + "|" + pad9_ 
	      + "|" + pad10_
		  ;

	//d("a3330 getTrxnData pad9_=[%s] pad10_=[%s]", s(pad9_), s(pad10_) );
}

void OmicroTrxn::print()
{
	// i("OmicroTrxn::print data=[%s]\n" );
}

// for testing only
void  OmicroTrxn::makeSimpleTrxn( const sstr &nodePubkey, 
		const sstr &userSecretKey, const sstr &userPublicKey,
		const sstr &from, const sstr &to, const sstr &amt)
{
	hdr_ = "IT";

	beacon_ = "12345678";
	//setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = from;
	receiver_ = to;

	amount_ = amt;

	setNowTimeStamp();

	trxntype_ = OM_PAYMENT; // payment

	assettype_ = "OC";
	setVoteInt(0);

	makeNodeSignature( nodePubkey );
	makeUserSignature( userSecretKey, userPublicKey );
}

void OmicroTrxn::makeNewAcctTrxn( const sstr &nodePubkey, 
			const sstr &userSecretKey, const sstr &userPublicKey ) 
{
	sstr userId;
	OmicroUserKey::getUserId( userPublicKey, userId );

	hdr_ = "IT";

	beacon_ = "12345678";
	//setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = userId;

	receiver_ = "NA";
	amount_ = "0";
	setNowTimeStamp();

	trxntype_ = OM_NEWACCT; // create acct
	assettype_ = "";
	setVoteInt(0);

	makeNodeSignature( nodePubkey );
	makeUserSignature( userSecretKey, userPublicKey );
}

void OmicroTrxn::makeUserSignature( const sstr &userSecretKey, const sstr &usrPubkey )
{
	d("a222101 makeUserSignature ...");
	sstr data;
	getTrxnData( data );
	OmicroUserKey::sign( data, userSecretKey, userSignature_);
	userPubkey_ = usrPubkey;

	// debug
	/***
	bool rc = OmicroUserKey::verify( userSignature_, usrPubkey );
	d("a222207 makeUserSignature verify rc=%d 0 is false", rc );
	d("a222207 usrPubkey=[%s]", s(usrPubkey) );
	d("a222207 userSecretKey=[%s]", s(userSecretKey) );
	d("a222207 userSignature_=[%s]", s(userSignature_) );
	d("a222207 trxndata=[%s]", s(data) );
	***/
}

