#ifndef _om_blockmgr_h_
#define  _om_blockmgr_h_

#include <unordered_map>
#include "omstore.h"
#include "omicrotrxn.h"

using OmstorePtr = OmStore*;
class BlockMgr
{
  public:
    BlockMgr();
	void setDataDir( const sstr &dataDir );
    ~BlockMgr();

	int saveTrxn( OmicroTrxn &trxn);
	void queryTrxn( const sstr &trxnid, sstr &res );

  protected:
    void initDirs();

  	sstr dataDir_;
	sstr getStoreFilePath( const sstr &trxnId );
  	const int DIR_LEVEL1_NUM = 1024; // do not change this
  	const int DIR_LEVEL2_NUM = 1024; // do not change this
  	const int DIR_HASH_SEED1 = 1579; // do not change this
  	const int DIR_HASH_SEED2 = 2593; // do not change this
	std::unordered_map<sstr, OmstorePtr> storeMap_;

};

#endif
