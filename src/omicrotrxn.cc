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
	hdr = "TT"; // plaintext, trxn
}

OmicroTrxn::OmicroTrxn( const char *str )
{
	OmStrSplit sp(str, '|');
	hdr = sp[0];
	id = sp[1];
	beacon = sp[2];
	srvport = sp[3];
	sender = sp[4];
	receiver = sp[5];
	amount = sp[6];
	timestamp = sp[7];
	trxntype = sp[8];
	assettype = sp[9];
	vote = sp[10];

	pad1 = sp[11];
	pad2 = sp[12];
	pad3 = sp[13];
	pad4 = sp[14];
	pad5 = sp[15];
	pad6 = sp[16];
	pad7 = sp[17];
	pad8 = sp[18];
	pad9 = sp[19];
	pad10 = sp[20];

	cipher = sp[21];
	signature = sp[22];
}

OmicroTrxn::~OmicroTrxn()
{
}

void OmicroTrxn::setInitTrxn()
{
	hdr[TRXN_HEADER_START] = 'I'; 
}

void OmicroTrxn::setNotInitTrxn()
{
	hdr[TRXN_HEADER_START] = 'N'; 
}

bool OmicroTrxn::isInitTrxn()
{
	if ( 'I' == hdr[TRXN_HEADER_START] ) {
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
    id = s;
}

//2nd byte
void  OmicroTrxn::setXit( Byte xit)
{
	hdr[TRXN_HEADER_START+1] = xit;
}

Byte OmicroTrxn::getXit()
{
	return hdr[TRXN_HEADER_START+1];
}

void OmicroTrxn::setBeacon()
{
	char s[7+1];
    ulong pm = ipow(10, 7);
    sprintf(s, "%*d", 7, int(time(NULL)%pm) );
	beacon = s;
}


double OmicroTrxn::getAmountDouble()
{
	double f = atof(amount.c_str());
	return f;
}

void OmicroTrxn::setNowTimeStamp()
{
	char tb[32];
    struct timeval now;
    gettimeofday( &now, NULL );
    ulong tot = now.tv_sec*1000000 + now.tv_usec;
    sprintf(tb, "%lu", tot );
	timestamp = tb;
}

ulong OmicroTrxn::getTimeStampUS()
{
	return atol(timestamp.c_str());
	// microseconds since epoch
}

int OmicroTrxn::getVoteInt()
{
	int num = atoi(vote.c_str());
	return num;
}

void OmicroTrxn::setVoteInt( int votes )
{
	char v[16];
	sprintf(v, "%d", votes );
	vote = v;
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


void OmicroTrxn::makeSignature( const sstr &pubKey)
{
	sstr data;
	getTrxnData( data );
	OmicroKey::sign( data, pubKey, cipher, signature );
}

void OmicroTrxn::allstr( sstr &alldata )
{
	sstr data;
	getTrxnData( data );
	alldata = data + "|" + cipher + "|" + signature;
}

void OmicroTrxn::getTrxnID( sstr &id )
{
	id = sender + timestamp;
}

bool OmicroTrxn::isValidClientTrxn( const sstr &secretKey)
{
	ulong trxnTime = getTimeStampUS();
	unsigned long nowt = getNowTimeUS();
	if ( nowt - trxnTime > 60000000 ) {
		// lag of 60 seconds
		i("a303376 warn isValidClientTrxn() nowt=%ld trxnTime=%ld more than 60 seconds", nowt, trxnTime);
		return false;
	}

	// signature verification
	sstr data;
	getTrxnData( data );
	bool rc = OmicroKey::verify(data, signature, cipher, secretKey);
	/**
	d("a22208 trxndata=[%s] len=%d  rc=%d", s(data), data.size(), rc );
	d("a22208 secretKey=[%s] len=%d", s(secretKey), secretKey.size() );
	d("a22208 cipher=[%s] len=%d", s(cipher), cipher.size() );
	d("a22208 signature=[%s] len=%d", s(signature), signature.size() );
	**/
	return rc;
}

void OmicroTrxn::getTrxnData( sstr &data )
{
	data = hdr +
	      + "|" + id 
	      + "|" + beacon 
	      + "|" + srvport 
	      + "|" + sender 
	      + "|" + receiver 
	      + "|" + amount 
	      + "|" + timestamp 
	      + "|" + trxntype 
	      + "|" + assettype 
	      + "|" + vote 
	      + "|" + pad1 
	      + "|" + pad2 
	      + "|" + pad3 
	      + "|" + pad4 
	      + "|" + pad5 
	      + "|" + pad6 
	      + "|" + pad7 
	      + "|" + pad8 
	      + "|" + pad9 
	      + "|" + pad10
		  ;
}

void OmicroTrxn::print()
{
	// i("OmicroTrxn::print data=[%s]\n" );
}

// for testing only
void  OmicroTrxn::makeDummyTrxn( const sstr &pubkey )
{
	hdr = "IT";

	beacon = "12345678";
	//setBeacon();
	srvport = "127.0.0.1:client";

	sender = "0xAdhfgOkfuetOjr";

	receiver = "0xBIhwvGkBj8";

	amount = "123456789.999999";

	setNowTimeStamp();

	trxntype = "AB";

	assettype = "XY";
	setVoteInt(0);

	makeSignature( pubkey );
}

