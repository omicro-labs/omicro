#include <malloc.h>
#include <string.h>
#include <iostream>
#include "omicrotrxn.h"

OmicroTrxn::OmicroTrxn()
{
	data_[TRXN_TOTAL_SZ+1] = '\0';
}

OmicroTrxn::OmicroTrxn( const char *str )
{
	int len = strlen(str);
	if ( len >= TRXN_TOTAL_SZ ) {
		memcpy(data_, str, TRXN_TOTAL_SZ);
	} else {
		std::cout << "E00001 OmicroTrxn::OmicroTrxn(s) s=[" << str << "] is too short" << std::endl;
	}
	data_[TRXN_TOTAL_SZ+1] = '\0';
}

OmicroTrxn::~OmicroTrxn()
{
}

// caller needs to free the pointer
char* OmicroTrxn::getHeader()
{
	int start = TRXN_HEADER_START;
	int sz = TRXN_HEADER_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setHeader( const char *s)
{
	int len = strlen(s);
	int start = TRXN_HEADER_START;
	int sz = TRXN_HEADER_SZ;
	if ( len != sz ) {
		std::cout << "E10000 OmicroTrxn::setHeader s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getBeacon()
{
	int start = TRXN_BEACON_START;
	int sz = TRXN_BEACON_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setBeacon( const char *s)
{
	int start = TRXN_BEACON_START;
	int sz = TRXN_BEACON_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10001 OmicroTrxn::setBeacon s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}


char* OmicroTrxn::getSender()
{
	int start = TRXN_SENDER_START;
	int sz = TRXN_SENDER_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setSender( const char *s)
{
	int start = TRXN_SENDER_START;
	int sz = TRXN_SENDER_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10002 OmicroTrxn::setSender s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getReceiver()
{
	int start = TRXN_RECEIVER_START;
	int sz = TRXN_RECEIVER_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setReceiver( const char *s)
{
	int start = TRXN_RECEIVER_START;
	int sz = TRXN_RECEIVER_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10003 OmicroTrxn::setReceiver s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

double OmicroTrxn::getAmountDouble()
{
	char *p = getAmount();
	double f = atof(p);
	free(p);
	return f;
}

char* OmicroTrxn::getAmount()
{
	int start = TRXN_AMOUNT_START;
	int sz = TRXN_AMOUNT_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setAmount( const char *s)
{
	int start = TRXN_AMOUNT_START;
	int sz = TRXN_AMOUNT_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10003 OmicroTrxn::setAmout s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getTimeStamp()
{
	int start = TRXN_TIMESTAMP_START;
	int sz = TRXN_TIMESTAMP_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setTimeStamp( const char *s)
{
	int start = TRXN_TIMESTAMP_START;
	int sz = TRXN_TIMESTAMP_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10004 OmicroTrxn::setTimeStamp s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}


char* OmicroTrxn::getTrxnType()
{
	int start = TRXN_TRXNTYPE_START;
	int sz = TRXN_TRXNTYPE_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setTrxnType( const char *s)
{
	int start = TRXN_TRXNTYPE_START;
	int sz = TRXN_TRXNTYPE_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10005 OmicroTrxn::setTrxnType s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

char* OmicroTrxn::getAssetType()
{
	int start = TRXN_ASSETTYPE_START;
	int sz = TRXN_ASSETTYPE_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setAssetType( const char *s)
{
	int start = TRXN_ASSETTYPE_START;
	int sz = TRXN_ASSETTYPE_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10006 OmicroTrxn::setAssetType s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}


char* OmicroTrxn::getSignature()
{
	int start = TRXN_SIGNATURE_START;
	int sz = TRXN_SIGNATURE_SZ;
	char *p = (char*)malloc(sz+1);
	memcpy( p, data_+start, sz );
	p[sz] = '\0';
	return p;
}

bool OmicroTrxn::setSignature( const char *s)
{
	int start = TRXN_SIGNATURE_START;
	int sz = TRXN_SIGNATURE_SZ;
	int len = strlen(s);
	if ( len != sz ) {
		std::cout << "E10007 OmicroTrxn::setSignature s=[" << s << "] wrong size" << std::endl; 
		return false;
	}
	memcpy( data_+start, s, sz );
	return true;
}

// caller frees the pointer
char *OmicroTrxn::getString()
{
	return strdup(data_);
}
