#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#include "omicrodef.h"

#define TRXN_HEADER_START   0
#define OM_PAYMENT         "P"
#define OM_NEWACCT         "A"
#define OM_GETBAL          "B"
#define OM_SMART_CONTRACT  "C"
#define OM_SECURE_CONTRACT "S"

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


	void allstr( sstr &alldata );

	void  getTrxnID( sstr &id);
	void  makeSimpleTrxn( const sstr &nodePubkey, const sstr &userSecretKey, const sstr &userPublicKey, 
						  const sstr &fromId, const sstr &toId, const sstr &amt );
	void  makeNewAcctTrxn( const sstr &nodePubkey, const sstr &userSecKey, const sstr &userPubkey );
	void  print();
	void  setInitTrxn();
	void  setNotInitTrxn();
	void  setXit( Byte xit);
	Byte  getXit();
	bool  isInitTrxn();
	bool  validateTrxn( const sstr &skey );
	void  getTrxnData( sstr &data );

	void makeNodeSignature( const sstr &nodePubKey );
	void makeUserSignature( const sstr &userSecretKey, const sstr &usrPubkey );

	// data members
	sstr hdr_;      // 6 bytes   0
	sstr id_;       // 1
	sstr beacon_;
	sstr srvport_;  // 3
	sstr sender_;
	sstr receiver_; // 5
	sstr amount_;
	sstr timestamp_; // 7
	sstr trxntype_;  // P: payment  A: user account creation
	sstr assettype_; // 9
	sstr pad1_;    // 10
	sstr pad2_;    // 11
	sstr pad3_;
	sstr pad4_;
	sstr pad5_;    // 14
	sstr pad6_;
	sstr pad7_;    // 16
	sstr pad8_;
	sstr pad9_;    // 18
	sstr pad10_;   // 19
	sstr cipher_;  // 20
	sstr signature_;  // 21
	sstr userPubkey_;  // 22
	sstr userSignature_;  // 23
	sstr vote_;    // 24
	sstr fence_;   // 25

};

#endif
