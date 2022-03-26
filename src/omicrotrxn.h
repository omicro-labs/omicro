#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#include "omicrodef.h"

#define TRXN_HEADER_START  0
#define TRXN_HEADER_SZ     6

#define TRXN_ID_START  (TRXN_HEADER_START + TRXN_HEADER_SZ)
#define TRXN_ID_SZ     8

#define TRXN_BEACON_START  (TRXN_ID_START + TRXN_ID_SZ)
#define TRXN_BEACON_SZ     8

#define TRXN_SRVPORT_START  (TRXN_BEACON_START+TRXN_BEACON_SZ)
#define TRXN_SRVPORT_SZ     26
// 26--->0 to disable SRVPORT field
// when adding a new field, update TRXN_DATA_SZ1 and TRXN_DATA_SZ2 below

#define TRXN_SENDER_START  (TRXN_SRVPORT_START+TRXN_SRVPORT_SZ)
//#define TRXN_SENDER_START  (TRXN_BEACON_START+TRXN_BEACON_SZ)
#define TRXN_SENDER_SZ     503

#define TRXN_RECEIVER_START (TRXN_SENDER_START+TRXN_SENDER_SZ)
#define TRXN_RECEIVER_SZ   503

#define TRXN_AMOUNT_START  (TRXN_RECEIVER_START+TRXN_RECEIVER_SZ)
#define TRXN_AMOUNT_SZ     16

#define TRXN_TIMESTAMP_START (TRXN_AMOUNT_START+TRXN_AMOUNT_SZ)
#define TRXN_TIMESTAMP_SZ  16

#define TRXN_TRXNTYPE_START (TRXN_TIMESTAMP_START+TRXN_TIMESTAMP_SZ)
#define TRXN_TRXNTYPE_SZ   2

#define TRXN_ASSETTYPE_START (TRXN_TRXNTYPE_START+TRXN_TRXNTYPE_SZ)
#define TRXN_ASSETTYPE_SZ  2

#define TRXN_VOTE_START (TRXN_ASSETTYPE_START+TRXN_ASSETTYPE_SZ)
#define TRXN_VOTE_SZ  10

#define TRXN_SIGNATURE_START  (TRXN_VOTE_START+TRXN_VOTE_SZ)
#define TRXN_SIGNATURE_SZ  64

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

	char *getHeader();
	bool setHeader( const char *s);

	char *getID();
	void setID();
	//void setBlankID();

	char *getBeacon();
	bool setBeacon( const char *s);
	bool setBeacon();

	char *getSrvPort();
	bool setSrvPort( const char *s );

	char *getSender();
	bool setSender( const char *s );

	char *getReceiver();
	bool setReceiver( const char *s );

	char *getAmount();
	double getAmountDouble();
	bool setAmount( const char *s );

	char *getTimeStamp();
	ulong getTimeStampUS();
	bool setTimeStamp( const char *s );
	bool setNowTimeStamp();

	char *getTrxnType();
	bool setTrxnType( const char *s );

	char *getAssetType();
	bool setAssetType( const char *s );

	char *getVote();
	int  getVoteInt();
	bool setVote( const char *s );
	bool setVoteInt( int votes );
	void addVote(int vote);
	void minusVote(int vote);

	char *getSignature();
	bool setSignature( const char *s );

	const char *getString() const;
	const char *str() const;
	char *getTrxnID() const;
	void  getTrxnIDStr( sstr &idstr ) const;
	int   length() const;
	int   size() const;
	void  makeDummyTrxn();
	void  print();
	bool  setInitTrxn();
	bool  setNotInitTrxn();
	bool  setXit( Byte xit);
	Byte  getXit();
	bool  isInitTrxn();
	bool  isValidClientTrxn();


  protected:
	char *data_;
	bool readOnly_;
};

#endif
