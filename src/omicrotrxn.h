#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#include "omicrodef.h"

#define TRXN_HEADER_START  0
#define TRXN_HEADER_SZ     6
#define TRXN_HEADER_IDX     0

#define TRXN_ID_START  (TRXN_HEADER_START + TRXN_HEADER_SZ)
#define TRXN_ID_SZ     8
#define TRXN_ID_IDX    1

#define TRXN_BEACON_START  (TRXN_ID_START + TRXN_ID_SZ)
#define TRXN_BEACON_SZ     8
#define TRXN_BEACON_IDX     2

#define TRXN_SRVPORT_START  (TRXN_BEACON_START+TRXN_BEACON_SZ)
#define TRXN_SRVPORT_SZ     26
#define TRXN_SRVPORT_IDX    3
// 26--->0 to disable SRVPORT field
// when adding a new field, update TRXN_DATA_SZ1 and TRXN_DATA_SZ2 below

#define TRXN_SENDER_START  (TRXN_SRVPORT_START+TRXN_SRVPORT_SZ)
//#define TRXN_SENDER_START  (TRXN_BEACON_START+TRXN_BEACON_SZ)
#define TRXN_SENDER_SZ     503
#define TRXN_SENDER_IDX    4

#define TRXN_RECEIVER_START (TRXN_SENDER_START+TRXN_SENDER_SZ)
#define TRXN_RECEIVER_SZ   503
#define TRXN_RECEIVER_IDX  5

#define TRXN_AMOUNT_START  (TRXN_RECEIVER_START+TRXN_RECEIVER_SZ)
#define TRXN_AMOUNT_SZ     16
#define TRXN_AMOUNT_IDX    6

#define TRXN_TIMESTAMP_START (TRXN_AMOUNT_START+TRXN_AMOUNT_SZ)
#define TRXN_TIMESTAMP_SZ  16
#define TRXN_TIMESTAMP_IDX 7

#define TRXN_TRXNTYPE_START (TRXN_TIMESTAMP_START+TRXN_TIMESTAMP_SZ)
#define TRXN_TRXNTYPE_SZ   2
#define TRXN_TRXNTYPE_IDX  8

#define TRXN_ASSETTYPE_START (TRXN_TRXNTYPE_START+TRXN_TRXNTYPE_SZ)
#define TRXN_ASSETTYPE_SZ  2
#define TRXN_ASSETTYPE_IDX 9

#define TRXN_VOTE_START (TRXN_ASSETTYPE_START+TRXN_ASSETTYPE_SZ)
#define TRXN_VOTE_SZ  10
#define TRXN_VOTE_IDX  10

#define TRXN_SIGNATURE_START  (TRXN_VOTE_START+TRXN_VOTE_SZ)
#define TRXN_SIGNATURE_SZ  64
#define TRXN_SIGNATURE_IDX 11

#define TRXN_DATA_SZ1  (TRXN_ID_SZ+TRXN_BEACON_SZ+TRXN_SRVPORT_SZ+TRXN_SENDER_SZ+TRXN_RECEIVER_SZ+TRXN_AMOUNT_SZ)
#define TRXN_DATA_SZ2  (TRXN_TIMESTAMP_SZ+TRXN_TRXNTYPE_SZ+TRXN_ASSETTYPE_SZ+TRXN_VOTE_SZ)
#define TRXN_DATA_SZ   (TRXN_DATA_SZ1 + TRXN_DATA_SZ2)

#define TRXN_BODY_SZ  (TRXN_HEADER_SZ+TRXN_DATA_SZ)
#define TRXN_TOTAL_SZ (TRXN_BODY_SZ+TRXN_SIGNATURE_SZ)

class OmicroTrxn
{
  public:
  	OmicroTrxn();
  	OmicroTrxn(const char *str);
  	~OmicroTrxn();

	char *getID();
	void setID();

	void setBeacon();

	/***
	char *getSrvPort();
	bool setSrvPort( const char *s );

	char *getSender();
	bool setSender( const char *s );

	char *getReceiver();
	bool setReceiver( const char *s );

	char *getAmount();
	**/

	double getAmountDouble();
	// bool setAmount( const char *s );

	//char *getTimeStamp();
	ulong getTimeStampUS();
	//bool setTimeStamp( const char *s );
	void setNowTimeStamp();

	/**
	char *getTrxnType();
	void getTrxnType( sstr &txnType );
	bool setTrxnType( const char *s );

	char *getAssetType();
	bool setAssetType( const char *s );
	**/

	//char *getVote();
	//bool setVote( const char *s );

	int  getVoteInt();
	void setVoteInt( int votes );
	void addVote(int vote);
	void minusVote(int vote);
	void makeSignature( const sstr &pubKey );

	//char *getSignature();
	//bool setSignature( const char *s );
	//const char *getString() const;

	sstr  &&str();

	void getTrxnID( sstr &id) const;
	//void  getTrxnIDStr( sstr &idstr ) const;
	//int   length() const;
	//int   size() const;
	void  makeDummyTrxn( const sstr &pubkey );
	void  print();
	void  setInitTrxn();
	void  setNotInitTrxn();
	void  setXit( Byte xit);
	Byte  getXit();
	bool  isInitTrxn();
	bool  isValidClientTrxn( const sstr &skey );
	void  getTrxnData( sstr &data ) const;


	// data members
	sstr hdr; // 6 bytes
	sstr id;
	sstr beacon;
	sstr srvport;
	sstr sender;
	sstr receiver;
	sstr amount;
	sstr timestamp;
	sstr trxntype;
	sstr assettype;
	sstr vote;
	sstr pad1;
	sstr pad2;
	sstr pad3;
	sstr pad4;
	sstr pad5;
	sstr pad6;
	sstr pad7;
	sstr pad8;
	sstr pad9;
	sstr pad10;
	sstr cipher;
	sstr signature;

};

#endif
