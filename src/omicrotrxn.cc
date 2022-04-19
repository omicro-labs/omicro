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
#include "omquery.h"
#include "omlog.h"
#include "omaccount.h"
#include "omjson.h"
#include "omlimits.h"
EXTERN_LOGGING

OmicroTrxn::OmicroTrxn()
{
	thdr_ = "TT"; // plaintext, trxn
}

OmicroTrxn::OmicroTrxn( const char *str )
{
	d("a23338 OmicroTrxn ctor from string");
	d("a23338 str=[%s]", str);

	int i=0;
	OmStrSplit sp(str, '|');
	//hdr_ = sp[i++]; 
	id_ = sp[i++];
	beacon_ = sp[i++];
	//srvport_ = sp[i++];
	sender_ = sp[i++];
	receiver_ = sp[i++];
	amount_ = sp[i++];
	timestamp_ = sp[i++];
	trxntype_ = sp[i++];
	assettype_ = sp[i++];
	request_ = sp[i++];

	pad1_ = sp[i++];
	pad2_ = sp[i++];
	pad3_ = sp[i++];
	pad4_ = sp[i++];
	pad5_ = sp[i++];
	pad6_ = sp[i++];
	pad7_ = sp[i++];
	pad8_ = sp[i++];
	pad9_ = sp[i++];
	pad10_ = sp[i++];

	//d("a33098 trxn str ctor pad9=[%s] pad10=[%s]", s(pad9_), s(pad10_) );

	// node PKI
	cipher_ = sp[i++];
	signature_ = sp[i++];

	// user account PKI
	userPubkey_ = sp[i++];
	userSignature_ = sp[i++];

	vote_ = sp[i++];
	fence_ = sp[i++];
	response_ = sp[i++];

	thdr_ = sp[i++]; 
	srvport_ = sp[i++];

	#ifdef OM_DEBUG
	nodepubkey_ = sp[i++];
	#endif

	if ( vote_.size() < 1 ) {
		vote_ = "0";
	}
}

OmicroTrxn::~OmicroTrxn()
{
}

void OmicroTrxn::setInitTrxn()
{
	thdr_[TRXN_HEADER_START] = 'I'; 
}

void OmicroTrxn::setNotInitTrxn()
{
	thdr_[TRXN_HEADER_START] = 'N'; 
}

bool OmicroTrxn::isInitTrxn()
{
	if ( 'I' == thdr_[TRXN_HEADER_START] ) {
		return true;
	} else {
		return false;
	}
}

void OmicroTrxn::setID()
{
	/***
    char s[16];
    std::uniform_int_distribution<uint64_t> distribution;
    std::mt19937_64   engine(std::random_device{}());
    uint64_t r1 = distribution(engine);
    sprintf(s, "%lx", r1 );
    id_ = s;
	***/
	id_ = "omtrxnid"; // todo
}

//2nd byte
void  OmicroTrxn::setXit( Byte xit)
{
	thdr_[TRXN_HEADER_START+1] = xit;
}

Byte OmicroTrxn::getXit()
{
	return thdr_[TRXN_HEADER_START+1];
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
	if ( amount_.size() < 1 ) return 0.0;
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
	if ( timestamp_.size() < 1 ) return 0;

	return atol(timestamp_.c_str());
	// microseconds since epoch
}

int OmicroTrxn::getVoteInt()
{
	if ( vote_.size() < 1 ) return 0;
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
	sstr trxndata;
	getTrxnData( trxndata );
	OmicroNodeKey::signSB3( trxndata, nodePubKey, cipher_, signature_ );

	#ifdef OM_DEBUG
	nodepubkey_ = nodePubKey;
	d("a222128 makeNodeSignature trxndata=[%s]", s(trxndata) );
	d("a222128 makeNodeSignature cipher=[%s]", s(cipher_) );
	d("a222128 makeNodeSignature signature=[%s]", s(signature_) );
	d("a222128 makeNodeSignature nodepubkey=[%s]", s(nodePubKey) );
	d("done makeNodeSignature\n");
	#endif
}

void OmicroTrxn::allstr( sstr &alldata )
{
	sstr data;
	getTrxnData( data );
	alldata = data + "|" + cipher_ + "|" + signature_ + "|" 
	          + userPubkey_ + "|" + userSignature_ + "|" + vote_ + "|" + fence_
			  + "|" + response_ + "|" + thdr_ + "|" + srvport_;

	#ifdef OM_DEBUG
	alldata += sstr("|") + nodepubkey_;
	#endif
}

void OmicroTrxn::getTrxnID( sstr &id )
{
	id = timestamp_ + ":" + sender_;
}

bool OmicroTrxn::validateTrxn( const sstr &secretKey )
{
	if ( sender_.size() > OM_NAME_MAXSZ ) {
		i("E203938 sender_ too long", s(sender_) );
		return false;
	}

	if ( receiver_.size() > OM_NAME_MAXSZ ) {
		i("E203934 receiver_ too long", s(receiver_) );
		return false;
	}

	ulong trxnTime = getTimeStampUS();
	unsigned long nowt = getNowTimeUS();
	if ( nowt - trxnTime > 60000000 ) {
		// lag of 60 seconds
		i("a303376 warn validateTrxn() nowt=%ld trxnTime=%ld more than 60 seconds", nowt, trxnTime);
		return false;
	}

	// signature verification
	sstr trxndata;
	getTrxnData( trxndata );
	bool rc = OmicroNodeKey::verifySB3( trxndata, signature_, cipher_, secretKey);
	if ( ! rc ) {
		i("E34408 node verify false");
		i("E34408 trxntype_=[%s]", s(trxntype_) );
		i("E34408 sender_=[%s] receiver_=[%s]", s(sender_), s(receiver_) );
		i("E34408 node secretkey=[%s]", s(secretKey) );
		#ifdef OM_DEBUG
		i("E34408 node pubkey=[%s]", s(nodepubkey_) );
		#endif
		i("E34408 data=[%s]", s(trxndata) );
		i("E34408 signature_=[%s]", s(signature_) );
		i("E34408 cipher_=[%s]", s(cipher_) );
		return false;
	}

	#ifdef OM_DEBUG
		i("E34408 node verify true");
		i("E34408 trxntype_=[%s]", s(trxntype_) );
		i("E34408 sender_=[%s] receiver_=[%s]", s(sender_), s(receiver_) );
		i("E34408 node secretkey=[%s]", s(secretKey) );
		i("E34408 node pubkey=[%s]", s(nodepubkey_) );
		i("E34408 data=[%s]", s(trxndata) );
		i("E34408 signature_=[%s]", s(signature_) );
		i("E34408 cipher_=[%s]", s(cipher_) );
	#endif


	rc = OmicroUserKey::verifyDL5(userSignature_, userPubkey_ );
	if ( ! rc ) {
		i("E34438 userkey verify false");
		return false;
	}
	/***
	d("a327639 OmicroUserKey::verify userSignature_=[%s]", s(userSignature_));
	d("a327639 OmicroUserKey::verify userPubkey_=[%s] rc=%d", s(userPubkey_), rc);
	***/

	if ( trxntype_ == OM_PAYMENT ) {
		double amt = getAmountDouble();
		if ( amt <= 0.0001 ) {
			i("E31538 amt=%.6g too small", amt );
			return false;
		}
	}

	return true;
}

void OmicroTrxn::getTrxnData( sstr &data )
{
	data = id_ 
	      + "|" + beacon_ 
	      + "|" + sender_ 
	      + "|" + receiver_ 
	      + "|" + amount_ 
	      + "|" + timestamp_ 
	      + "|" + trxntype_ 
	      + "|" + assettype_ 
	      + "|" + request_ 
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

// Send payment from one user to another
void  OmicroTrxn::makeSimpleTrxn( const sstr &nodePubkey, 
		const sstr &userSecretKey, const sstr &userPublicKey,
		const sstr &from, const sstr &to, const sstr &amt)
{
	thdr_ = "IT";
	setID();

	setBeacon();
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
			const sstr &userSecretKey, const sstr &userPublicKey, const sstr &userName ) 
{
	sstr userId;
	if ( userName.size() < 1 ) {
		OmicroUserKey::getUserId( userPublicKey, userId );
	} else {
		userId = userName;
	}
	d("a43071 makeNewAcctTrxn userId=[%s]", s(userId) );

	thdr_ = "IT";
	setID();

	setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = userId;

	receiver_ = "";
	amount_ = "0";
	setNowTimeStamp();

	trxntype_ = OM_NEWACCT; // create acct
	assettype_ = "";
	setVoteInt(0);

	// set request_
	OmJson js;
	js.add("ACTYPE", OM_ACCT_USER);
	js.json( request_ );

	makeNodeSignature( nodePubkey );
	makeUserSignature( userSecretKey, userPublicKey );
}

void OmicroTrxn::makeNewTokenTrxn( const sstr &nodePubkey, 
			const sstr &userSecretKey, const sstr &userPublicKey, 
			const sstr &ownerId, const sstr &tokenSpec ) 
{
	d("a41071 makeNewTokenTrxn ownerId=[%s]", s(ownerId) );

	thdr_ = "IT";
	setID();

	setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = ownerId;

	receiver_ = "";
	amount_ = "0";
	setNowTimeStamp();

	trxntype_ = OM_NEWTOKEN; // create acct
	assettype_ = "T";
	setVoteInt(0);

	// set request_
	request_ = tokenSpec;

	makeNodeSignature( nodePubkey );
	makeUserSignature( userSecretKey, userPublicKey );
}

void OmicroTrxn::makeUserSignature( const sstr &userSecretKey, const sstr &usrPubkey )
{
	d("a222101 makeUserSignature ...");
	sstr data;
	getTrxnData( data );
	OmicroUserKey::signDL5( data, userSecretKey, userSignature_);
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

void OmicroTrxn::makeAcctQuery( const sstr &nodePubkey, const sstr &secretKey, 
								const sstr &publicKey, const sstr &fromId )
{
	thdr_ = "IT";
	setID();
	setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = fromId;
	setNowTimeStamp();
	trxntype_ = OM_QUERY; 

	OmQuery q;
	q.addField("balance");
	q.addField("out");
	q.addField("in");
	q.addField("tokentype");
	q.addField("keytype");
	q.addField("accttype");
	sstr qstr;
	q.json( qstr );
	request_ = qstr;

	setVoteInt(0);
	makeNodeSignature( nodePubkey );
	makeUserSignature( secretKey, publicKey );
}

void OmicroTrxn::makeTokensQuery( const sstr &nodePubkey, const sstr &secretKey, 
								const sstr &publicKey, const sstr &fromId )
{
	thdr_ = "IT";
	setID();
	setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = fromId;
	setNowTimeStamp();
	trxntype_ = OM_QUERY; 

	OmQuery q;
	q.addField("balance");
	q.addField("out");
	q.addField("in");
	q.addField("tokentype");
	q.addField("keytype");
	q.addField("accttype");
	q.addField("tokens");
	sstr qstr;
	q.json( qstr );
	request_ = qstr;

	setVoteInt(0);
	makeNodeSignature( nodePubkey );
	makeUserSignature( secretKey, publicKey );
}

void OmicroTrxn::makeOneTokenQuery( const sstr &nodePubkey, const sstr &secretKey, 
								const sstr &publicKey, const sstr &fromId, const sstr &tokenName  )
{
	thdr_ = "IT";
	setID();
	setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = fromId;
	setNowTimeStamp();
	trxntype_ = OM_QUERY; 

	OmQuery q;
	q.predicate_ = sstr("name=") + tokenName;
	//q.addField("balance");
	//q.addField("out");
	//q.addField("in");
	//q.addField("tokentype");
	q.addField("keytype");
	q.addField("accttype");
	//q.addField("tokens");
	q.addField("token");
	sstr qstr;
	q.json( qstr );
	request_ = qstr;

	setVoteInt(0);
	makeNodeSignature( nodePubkey );
	makeUserSignature( secretKey, publicKey );
}

// Send tokens from one account to another
void  OmicroTrxn::makeTokenTransfer( const sstr &nodePubkey, 
		const sstr &userSecretKey, const sstr &userPublicKey,
		const sstr &from, const sstr &tokenJson, const sstr &to, const sstr &amt)
{
	thdr_ = "IT";
	setID();

	setBeacon();
	srvport_ = "127.0.0.1:client";

	sender_ = from;
	receiver_ = to;

	amount_ = amt;

	setNowTimeStamp();

	trxntype_ = OM_XFERTOKEN; // transfer tokens
	request_ = tokenJson;

	assettype_ = "TK";
	setVoteInt(0);

	makeNodeSignature( nodePubkey );
	makeUserSignature( userSecretKey, userPublicKey );
}
