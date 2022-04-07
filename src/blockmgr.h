#ifndef _om_blockmgr_h_
#define  _om_blockmgr_h_

#include <stdio.h>
#include <unordered_map>
#include "omstore.h"
#include "omicrotrxn.h"
#include "omutil.h"

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

  protected:
    void initDirs();
	int saveTrxn( OmicroTrxn &trxn);
	FILE *appendToBlockchain( OmicroTrxn &t, const sstr &userId, char ttype, const sstr &ymdh );
	void rollbackFromBlockchain( OmicroTrxn &t, const sstr &userId, const sstr &yyyymmddhh );
	int updateAcctBalances( OmicroTrxn &trxn);
	int createAcct( OmicroTrxn &trxn);
	void markBlockchain(FILE *fp, OmicroTrxn &t, const sstr &userId, char stat );
	int readTrxns(const sstr &from, const sstr &timestamp, const sstr &trxnId, std::vector<sstr> &rec, char &tstat, sstr &err );

	sstr getUserPath( const sstr &userId );
	sstr getAcctStoreFilePath( const sstr &userId);
	char *findSaveStore( const sstr &userId, OmstorePtr &ptr );


  	sstr dataDir_;
  	const int DIR_LEVEL1_NUM = 1019; // do not change this
  	const int DIR_LEVEL2_NUM = 1019; // do not change this
  	const int DIR_HASH_SEED1 = 1579; // do not change this
  	const int DIR_HASH_SEED2 = 2593; // do not change this
	std::unordered_map<sstr, OmstorePtr> trxnStoreMap_;
	std::unordered_map<sstr, OmstorePtr> acctStoreMap_;

};

#endif
