#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

#include <string>

#define TRXN_HEADER_START   0
#define OM_PAYMENT         "P"
#define OM_NEWACCT         "A"
#define OM_NEWTOKEN        "T"
#define OM_XFERTOKEN       "X"
#define OM_GETBAL          "B"
#define OM_SMART_CONTRACT  "C"
#define OM_SECURE_CONTRACT "S"
#define OM_QUERY           "Q"

//#define OM_DEBUG           1

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
	void  makeSimpleTrxn( const std::string &nodePubkey, const std::string &userSecretKey, 
						  const std::string &userPublicKey, const std::string &fromId, 
						  const std::string &toId, const std::string &amt );
	void  makeNewAcctTrxn( const std::string &nodePubkey, const std::string &userSecKey, 
						   const std::string &userPubkey, const std::string &userId );
	void  makeNewTokenTrxn( const std::string &nodePubkey, const std::string &userSecKey, 
						   const std::string &userPubkey, const std::string &ownerId,
						   const std::string &tokenSpec );
	void  makeAcctQuery( const std::string &nodePubkey, const std::string &secretKey, 
						 const std::string &publicKey, const std::string &fromId );
	void  makeTokensQuery( const std::string &nodePubkey, const std::string &secretKey, 
						 const std::string &publicKey, const std::string &fromId );
	void  makeOneTokenQuery( const std::string &nodePubkey, const std::string &secretKey, 
						 const std::string &publicKey, const std::string &fromId, const std::string &token );

	void  makeTokenTransfer( const std::string &nodePubkey, const std::string &userSecretKey, 
						      const std::string &userPublicKey, 
						      const std::string &fromId, const std::string &tokenJson, 
							  const std::string &toId, const std::string &amt );

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
	// If order is changed, update BlockMgr::readTrxns() too
	// trxn data
	std::string id_;
	std::string beacon_;
	std::string sender_;
	std::string receiver_;
	std::string amount_;
	std::string timestamp_;
	std::string trxntype_;       // P: payment  A: user account creation
	std::string assettype_;
	std::string request_;
	std::string pad1_;
	std::string pad2_;
	std::string pad3_;
	std::string pad4_;
	std::string pad5_;
	std::string pad6_;
	std::string pad7_;
	std::string pad8_;
	std::string pad9_;
	std::string pad10_;

	// non-trxn data
	std::string cipher_;
	std::string signature_;
	std::string userPubkey_;
	std::string userSignature_;
	std::string vote_;
	std::string fence_;
	std::string response_;

	std::string thdr_;
	std::string srvport_;

	#ifdef OM_DEBUG
	std::string nodepubkey_;      // 28  for debug only
	#endif
};

#endif
