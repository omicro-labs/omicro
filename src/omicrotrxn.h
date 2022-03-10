#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#define TRXN_HEADER_START  0
#define TRXN_HEADER_SZ     6

#define TRXN_BEACON_START  TRXN_HEADER_SZ
#define TRXN_BEACON_SZ     8

#define TRXN_SENDER_START  (TRXN_BEACON_START+TRXN_BEACON_SZ)
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

#define TRXN_SIGNATURE_START  (TRXN_ASSETTYPE_START+TRXN_ASSETTYPE_SZ)
#define TRXN_SIGNATURE_SZ  64

#define TRXN_DATA_SZ1  (TRXN_BEACON_SZ+TRXN_SENDER_SZ+TRXN_RECEIVER_SZ+TRXN_AMOUNT_SZ)
#define TRXN_DATA_SZ2  (TRXN_TIMESTAMP_SZ+TRXN_TRXNTYPE_SZ+TRXN_ASSETTYPE_SZ)
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

	char *getBeacon();
	bool setBeacon( const char *s);

	char *getSender();
	bool setSender( const char *s );

	char *getReceiver();
	bool setReceiver( const char *s );

	char *getAmount();
	double getAmountDouble();
	bool setAmount( const char *s );

	char *getTimeStamp();
	bool setTimeStamp( const char *s );

	char *getTrxnType();
	bool setTrxnType( const char *s );

	char *getAssetType();
	bool setAssetType( const char *s );

	char *getSignature();
	bool setSignature( const char *s );

	const char *getString();
	const char *str();
	char *getTrxnID();
	int   length();
	int   size();
	void  makeDummyTrxn();
	void  print();
	bool  setInitTrxn();
	bool  isInitTrxn();
	bool  isValidClientTrxn();


  protected:
	char *data_;
	bool readOnly_;
  	/*** Inside data_:
	header_[TRXN_HEADER_SZ];
	beacon_[TRXN_BEACON_SZ];
	sender_[TRXN_SENDER_SZ];
	receiver_[TRXN_RECEIVER_SZ];
	amount_[TRXN_AMOUNT_SZ];   // "123456789.123456"
	timestamp_[TRXN_TIMESTAMP_SZ];  // "123456789012.124"
	trxnType_[TRXN_TRXNTYPE_SZ];
	assetType_[TRXN_ASSETTYPE_SZ];
	signature_[TRXN_SIGNATURE_SZ];
	***/
};

#endif
