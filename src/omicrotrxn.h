#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#include <string>

#define TRXN_HEADER_START   0
#define OM_PAYMENT         "P"
#define OM_NEWACCT         "A"
#define OM_GETBAL          "B"
#define OM_SMART_CONTRACT  "C"
#define OM_SECURE_CONTRACT "S"
#define OM_QUERY           "Q"

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
	unsigned long getTimeStampUS();
	void setNowTimeStamp();

	int  getVoteInt();
	void setVoteInt( int votes );
	void addVote(int vote);
	void minusVote(int vote);

	void allstr( std::string &alldata );

	void  getTrxnID( std::string &id);
	void  makeSimpleTrxn( const std::string &nodePubkey, const std::string &userSecretKey, const std::string &userPublicKey, 
						  const std::string &fromId, const std::string &toId, const std::string &amt );
	void  makeNewAcctTrxn( const std::string &nodePubkey, const std::string &userSecKey, const std::string &userPubkey );
	void  makeAcctQuery( const std::string &nodePubkey, const std::string &secretKey, const std::string &publicKey, const std::string &fromId );

	void  print();
	void  setInitTrxn();
	void  setNotInitTrxn();
	void  setXit( unsigned char xit);
	unsigned char  getXit();
	bool  isInitTrxn();
	bool  validateTrxn( const std::string &skey );
	void  getTrxnData( std::string &data );

	void makeNodeSignature( const std::string &nodePubKey );
	void makeUserSignature( const std::string &userSecretKey, const std::string &usrPubkey );

	// data members
	std::string hdr_;            // 0
	std::string id_;             // 1
	std::string beacon_;         // 2
	std::string srvport_;        // 3
	std::string sender_;         // 4
	std::string receiver_;       // 5
	std::string amount_;         // 6
	std::string timestamp_;      // 7
	std::string trxntype_;       // P: payment  A: user account creation
	std::string assettype_;      // 9
	std::string request_;        // 10
	std::string pad1_;           // 11
	std::string pad2_;           // 12
	std::string pad3_;           // 13
	std::string pad4_;           // 14
	std::string pad5_;           // 15
	std::string pad6_;           // 16
	std::string pad7_;           // 17
	std::string pad8_;           // 18
	std::string pad9_;           // 19
	std::string pad10_;          // 20
	std::string cipher_;         // 21
	std::string signature_;      // 22
	std::string userPubkey_;     // 23
	std::string userSignature_;  // 24
	std::string vote_;           // 25
	std::string fence_;          // 26
	std::string response_;       // 27

};

#endif
