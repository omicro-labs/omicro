#ifndef _om_blockmgr_h_
#define  _om_blockmgr_h_

#include <stdio.h>
#include <unordered_map>
#include "omstore.h"
#include "omicrotrxn.h"
#include "omutil.h"
#include "trxnlist.h"

#define OM_MEMPOOL_SZ 3000

using OmstorePtr = OmStore*;
class BlockMgr
{
  public:
    BlockMgr();
	void setDataDir( const sstr &dataDir );
    ~BlockMgr();

	int receiveTrxn( OmicroTrxn &trxn);
	void queryTrxn( const sstr &from, const sstr &trxnId, const sstr &timestamp, sstr &res );
	double getBalance( const sstr &from ) ;
	int    getBalanceAndPubkey( const sstr &from, double &bal, sstr &pubkey );
	void   getFence( const sstr &from, sstr &fence );
	int    runQuery( OmicroTrxn &trxn, sstr &res );
	void   getTokens( const sstr &from, sstr &tokens ) ;
	int    isXferTokenValid( OmicroTrxn &trxn);

  protected:
    void initDirs();
	int saveTrxn( OmicroTrxn &trxn);
	FILE *appendToBlockchain( OmicroTrxn &t, const sstr &userId, char ttype, const sstr &ymdh );
	void rollbackFromBlockchain( OmicroTrxn &t, const sstr &userId, const sstr &yyyymmddhh );
	int updateAcctBalances( OmicroTrxn &trxn);
	int createAcct( OmicroTrxn &trxn);
	int createToken( OmicroTrxn &trxn);
	int transferToken( OmicroTrxn &trxn);
	void markBlockchain(FILE *fp, OmicroTrxn &t, const sstr &userId, char stat );
	int readTrxns(const sstr &from, const sstr &timestamp, const sstr &trxnId, std::vector<sstr> &rec, char &tstat, sstr &err );

	sstr getUserPath( const sstr &userId );
	sstr getAcctStoreFilePath( const sstr &userId);
	char *findSaveStore( const sstr &userId, OmstorePtr &ptr );

	int  validateReqTokens( const sstr &from, sstr &requestJson );
	void saveTrxnList( const sstr &from, const sstr &timestamp ); 
	int  checkValidTokens( const sstr &from, const sstr &to, const sstr &reqJson, sstr &fromTokens, sstr &toTokens);
	int  modifyTokens( const sstr &from, const sstr &to, const sstr &reqJson, sstr &fromTokens, sstr &toTokens);

  	sstr dataDir_;
  	const int DIR_LEVEL1_NUM = 1019; // do not change this
  	const int DIR_LEVEL2_NUM = 1019; // do not change this
  	const int DIR_HASH_SEED1 = 1579; // do not change this
  	const int DIR_HASH_SEED2 = 2593; // do not change this
	std::unordered_map<sstr, OmstorePtr> trxnStoreMap_;
	std::unordered_map<sstr, OmstorePtr> acctStoreMap_;
	TrxnList  trxnList_;
};

#endif
