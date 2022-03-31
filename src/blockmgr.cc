#include <sys/stat.h>
#include "blockmgr.h"
#include "omutil.h"
#include "xxHash/xxhash.h"
EXTERN_LOGGING


BlockMgr::BlockMgr()
{
}

void BlockMgr::setDataDir(  const sstr &dataDir )
{
	dataDir_ = dataDir;
	initDirs();
	d("a4447 initDirs done");
}

BlockMgr::~BlockMgr()
{
}

int BlockMgr::saveTrxn( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	auto itr = storeMap_.find( trxnId );
	if ( itr == storeMap_.end() ) {
		// omstore is created yet, create it and save trxn
		sstr fpath = getStoreFilePath( trxnId );
		OmstorePtr ptr = new OmStore( fpath.c_str(), OM_DB_WRITE );
		sstr &&ts = trxn.str();
		ptr->put( trxnId.c_str(), trxnId.size(),  ts.c_str(), ts.size() );
		storeMap_.emplace( trxnId, ptr );
	} else {
		sstr &&ts = trxn.str();
		itr->second->put( trxnId.c_str(), trxnId.size(),  ts.c_str(), ts.size() );
	}

	return 0;
}

void BlockMgr::queryTrxn( const sstr &trxnId, sstr &res )
{
	auto itr = storeMap_.find( trxnId );
	if ( itr == storeMap_.end() ) {
		res = trxnId + "|NOTFOUND1";
		return;
	} else {
		char *p = itr->second->get( trxnId.c_str() ); 
		if ( NULL == p ) {
			res = trxnId + "|NOTFOUND2";
			return; 
		} else {
			res = sstr(p);
			return;
		}
	}
}


void BlockMgr::initDirs()
{
	makedirPath( dataDir_ );
	d("a44427 mkdir [%s]", dataDir_.c_str() );
	sstr topd = dataDir_;
	sstr is, js, d;

	sstr d1, d2, d3;
    d1 = topd + "/23/45";
    d2 = topd + "/408/23";
    d3 = topd + "/278/785";
	if ( 0 == ::access(d1.c_str(), R_OK|W_OK ) 
	     && 0 == ::access(d2.c_str(), R_OK|W_OK )
		 && 0 == ::access(d3.c_str(), R_OK|W_OK ) )
	{
		i("initDirs() was already done");
		return;
	}

    for ( int i=0; i < DIR_LEVEL1_NUM; ++i ) {
        is = std::to_string(i);
        d = topd + "/" + is;
        ::mkdir( d.c_str(), 0700 );
        for ( int j=0; j < DIR_LEVEL2_NUM; ++j ) {
            js = std::to_string(j);
            d = topd + "/" + is + "/" + js;
           	::mkdir( d.c_str(), 0700 );
        }
    }
}

sstr BlockMgr::getStoreFilePath( const sstr &trxnId )
{
	XXH64_hash_t hash1 = XXH64( trxnId.c_str(), trxnId.size(), DIR_HASH_SEED1 ) % DIR_LEVEL1_NUM;
	XXH64_hash_t hash2 = XXH64( trxnId.c_str(), trxnId.size(), DIR_HASH_SEED2 ) % DIR_LEVEL2_NUM;

	sstr fpath = dataDir_ + "/" + std::to_string(hash1) + "/" + std::to_string(hash2) + "/omstore.hdb";
	return fpath;
}
