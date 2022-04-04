#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#include "omicrodef.h"

#define TRXN_HEADER_START  0

class OmicroTrxn
{
  public:
  	OmicroTrxn();
  	OmicroTrxn(const char *str);
  	~OmicroTrxn();

	char *getID();
	void setID();

	void setBeacon();
	double getAmountDouble();
	ulong getTimeStampUS();
	void setNowTimeStamp();

	int  getVoteInt();
	void setVoteInt( int votes );
	void addVote(int vote);
	void minusVote(int vote);
	void makeNodeSignature( const sstr &nodePubKey );

	void allstr( sstr &alldata );

	void  getTrxnID( sstr &id);
	void  makeDummyTrxn( const sstr &pubkey );
	void  print();
	void  setInitTrxn();
	void  setNotInitTrxn();
	void  setXit( Byte xit);
	Byte  getXit();
	bool  isInitTrxn();
	bool  isValidClientTrxn( const sstr &skey );
	void  getTrxnData( sstr &data );


	// data members
	sstr hdr;      // 6 bytes   0
	sstr id;       // 1
	sstr beacon;
	sstr srvport;
	sstr sender;
	sstr receiver; // 5
	sstr amount;
	sstr timestamp;
	sstr trxntype;  // P: payment  A: user account creation
	sstr assettype;
	sstr vote;    // 10
	sstr pad1;    // 11
	sstr pad2;    // 12
	sstr pad3;
	sstr pad4;
	sstr pad5;    // 15
	sstr pad6;
	sstr pad7;
	sstr pad8;
	sstr pad9;
	sstr pad10;   // 20
	sstr cipher;  // 21
	sstr signature;  // 22
	sstr userPubkey;  // 23
	sstr userSignature;  // 24

};

#endif
