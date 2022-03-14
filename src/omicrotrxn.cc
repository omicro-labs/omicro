#include <malloc.h>
#include <string.h>
#include <iostream>
#include "omicrotrxn.h"

OmicroTrxn::OmicroTrxn()
{
	data_ = (char*)malloc(TRXN_TOTAL_SZ+1);
	data_[TRXN_TOTAL_SZ] = '\0';
	readOnly_ = false;
}

OmicroTrxn::OmicroTrxn( const char *str )
{
	readOnly_ = true;
	int len = strlen(str);
	if ( len == TRXN_TOTAL_SZ ) {
		data_ = (char*)str;
	} else {
		std::cout << "E00001 OmicroTrxn::OmicroTrxn(s) s=[" << str << "] is too short" << std::endl;
		data_ = NULL;
	}
	data_[TRXN_TOTAL_SZ] = '\0';
}

OmicroTrxn::~OmicroTrxn()
{
	if ( ! readOnly_ ) {
		free( data_ );
	}
}

// caller needs to free the pointer
char* OmicroTrxn::getHeader()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_HEADER_START;
	int sz = TRXN_HEADER_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setHeader( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10010 OmicroTrxn::setHeader data_ is NULL" << std::endl; 
		return false;
	}

	int len = strlen(s);
	int start = TRXN_HEADER_START;
	int sz = TRXN_HEADER_SZ;
	if ( len != sz ) {
		std::cout << "E10000 OmicroTrxn::setHeader s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

bool  OmicroTrxn::setInitTrxn()
{
	if ( NULL == data_ ) {
		std::cout << "E10010 OmicroTrxn::setHeader data_ is NULL" << std::endl; 
		return false;
	}

	data_[TRXN_HEADER_START] = 'I'; 
	return true;
}

// TRXN_HEADER_START+0:  'C' initiated from client, 'L' from leader
bool OmicroTrxn::isInitTrxn()
{
	if ( NULL == data_ ) {
		std::cout << "E10110 OmicroTrxn::isInitTrxn data_ is NULL" << std::endl; 
		return false;
	}

	if ( 'I' == data_[TRXN_HEADER_START] ) {
		return true;
	} else {
		return false;
	}
}

bool  OmicroTrxn::setXit( Byte xit)
{
	if ( NULL == data_ ) {
		std::cout << "E10110 OmicroTrxn::setXit data_ is NULL" << std::endl; 
		return false;
	}

	data_[TRXN_HEADER_START+1] = xit;
	return true;
}

Byte OmicroTrxn::getXit()
{
	if ( NULL == data_ ) {
		std::cout << "E10112 OmicroTrxn::getXit data_ is NULL" << std::endl; 
		return 0;
	}
	return data_[TRXN_HEADER_START+1];
}

char* OmicroTrxn::getBeacon()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_BEACON_START;
	int sz = TRXN_BEACON_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setBeacon( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10011 OmicroTrxn::setBeacon data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_BEACON_START;
	int sz = TRXN_BEACON_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10001 OmicroTrxn::setBeacon s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

bool OmicroTrxn::setBeacon()
{
	if ( NULL == data_ ) {
		std::cout << "E10111 OmicroTrxn::setBeacon data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_BEACON_START;
	int sz = TRXN_BEACON_SZ;
	char s[TRXN_BEACON_SZ+1];
	sprintf(s, "%*d", TRXN_BEACON_SZ, int(time(NULL)%TRXN_BEACON_SZ) );

	memcpy( data_+start, s, sz );
	return true;
}


char* OmicroTrxn::getSender()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_SENDER_START;
	int sz = TRXN_SENDER_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setSender( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10012 OmicroTrxn::setSender data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_SENDER_START;
	int sz = TRXN_SENDER_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10002 OmicroTrxn::setSender s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getReceiver()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_RECEIVER_START;
	int sz = TRXN_RECEIVER_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setReceiver( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10013 OmicroTrxn::setReceiver data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_RECEIVER_START;
	int sz = TRXN_RECEIVER_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10003 OmicroTrxn::setReceiver s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

double OmicroTrxn::getAmountDouble()
{
	if ( NULL == data_ ) return 0.0;
	char *p = getAmount();
	double f = atof(p);
	free(p);
	return f;
}

char* OmicroTrxn::getAmount()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_AMOUNT_START;
	int sz = TRXN_AMOUNT_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setAmount( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10014 OmicroTrxn::setAmount data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_AMOUNT_START;
	int sz = TRXN_AMOUNT_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10004 OmicroTrxn::setAmout s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getTimeStamp()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_TIMESTAMP_START;
	int sz = TRXN_TIMESTAMP_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setTimeStamp( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10015 OmicroTrxn::setTimeStamp data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_TIMESTAMP_START;
	int sz = TRXN_TIMESTAMP_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10005 OmicroTrxn::setTimeStamp s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}


char* OmicroTrxn::getTrxnType()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_TRXNTYPE_START;
	int sz = TRXN_TRXNTYPE_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setTrxnType( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10016 OmicroTrxn::setTrxnType data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_TRXNTYPE_START;
	int sz = TRXN_TRXNTYPE_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10006 OmicroTrxn::setTrxnType s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getAssetType()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_ASSETTYPE_START;
	int sz = TRXN_ASSETTYPE_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setAssetType( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10017 OmicroTrxn::setAssetType data_ is NULL" << std::endl; 
		return false;
	}

	int start = TRXN_ASSETTYPE_START;
	int sz = TRXN_ASSETTYPE_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10007 OmicroTrxn::setAssetType s=[" << s << "] wrong size" << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}


char* OmicroTrxn::getSignature()
{
	if ( NULL == data_ ) return NULL;
	int start = TRXN_SIGNATURE_START;
	int sz = TRXN_SIGNATURE_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setSignature( const char *s)
{
	if ( NULL == data_ ) {
		std::cout << "E10018 OmicroTrxn::setSignature data_ is NULL" << std::endl; 
		return false;
	}
	int start = TRXN_SIGNATURE_START;
	int sz = TRXN_SIGNATURE_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10008 OmicroTrxn::setSignature s=[" << s << "] wrong size " << len << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

const char *OmicroTrxn::getString()
{
	return (char*)data_;
}
const char *OmicroTrxn::str()
{
	return (char*)data_;
}

char * OmicroTrxn::getTrxnID()
{
	// sender + timestamp
	char *p = (char*)malloc( TRXN_SENDER_SZ + TRXN_TIMESTAMP_SZ + 1);
	memcpy( p, data_ + TRXN_SENDER_START, TRXN_SENDER_SZ );
	memcpy( p + TRXN_SENDER_SZ, data_ + TRXN_TIMESTAMP_START, TRXN_TIMESTAMP_SZ );
	p[TRXN_SENDER_SZ + TRXN_TIMESTAMP_SZ] = '\0';
	return p;
}

int OmicroTrxn::length()
{
	return TRXN_TOTAL_SZ;
}

int OmicroTrxn::size()
{
	return TRXN_TOTAL_SZ;
}

bool OmicroTrxn::isValidClientTrxn()
{
	return true;
}

void OmicroTrxn::print()
{
	printf("OmicroTrxn::print data=[%s]\n", data_ );
	fflush(stdout);
}

// for testing only
void  OmicroTrxn::makeDummyTrxn()
{
	setHeader("123456");
	// setBeacon("12345678");
	setBeacon();

	setSender("0xAduehHhfjOkfjetOjrUrjQjfSfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaDDExkYm");

	setReceiver("0xBdHehIhwjGkJjSjBj8i0jVjPafEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjYkGjejrBrirJijXjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjeqrArirjrmfhwEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaleeeyxelkdppwsxn0xAduehfhfjfkfjejrjrirjrjfjfEyehxnckfhe038ejdskaBwEykZv");

	setAmount("123456789.999999");

	setTimeStamp("123456789012.999");

	setTrxnType("AB");

	setAssetType("XY");

	setSignature("SGBdbehZhIjfkVjegrqBiGjr3AqfEyehxnckfhe038ejdskaleeeyxelkdUpwsgg");
}


