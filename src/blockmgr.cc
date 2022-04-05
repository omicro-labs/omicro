#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "xxHash/xxhash.h"
#include "blockmgr.h"
#include "omutil.h"
#include "omstrsplit.h"
#include "omaccount.h"
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

int BlockMgr::receiveTrxn( OmicroTrxn &trxn)
{
	sstr yyyymmddhh = getYYYYMMDDHHFromTS(trxn.timestamp_);
	d("a65701 receiveTrxn ...");

	FILE *fp1 = appendToBlockchain(trxn, trxn.sender_, '-', yyyymmddhh);
	if ( NULL == fp1 ) {
		return -1;
	}

	FILE *fp2 = appendToBlockchain(trxn, trxn.receiver_, '+', yyyymmddhh);
	if ( NULL == fp2 ) {
		fclose( fp1 );
		rollbackFromBlockchain(trxn, trxn.sender_, yyyymmddhh );
		return -2;
	}

	int urc;
	if ( trxn.trxntype_ == OM_PAYMENT ) {
		urc = updateAcctBalances(trxn);
	} else if ( trxn.trxntype_ == OM_NEWACCT ) {
		urc = createAcct(trxn);
	} else {
		urc = -10;
	}

	if ( urc < 0 ) {
		// mark log failure 'F'
		fprintf(fp1, "F~");
		fprintf(fp2, "F~");
	} else {
		// mark log success 'T'
		fprintf(fp1, "T~");
		fprintf(fp2, "T~");
	}

	fclose(fp1);
	fclose(fp2);

	return 0;
}

int BlockMgr::createAcct( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	d("a32047 createAcct trxnId=[%s] from=[%s]", s(trxnId), s(from) );

	OmstorePtr srcptr;
	sstr fpath;
	sstr ts; trxn.getTrxnData(ts);

	auto itr1 = acctStoreMap_.find( from );
	if ( itr1 == acctStoreMap_.end() ) {
		fpath = getAcctStoreFilePath( from );
		srcptr = new OmStore( fpath.c_str(), OM_DB_WRITE );

		OmAccount acct;
		acct.balance_ = "0";
		acct.pubkey_ = trxn.userPubkey_;
		sstr rec;
		acct.str( rec );

		srcptr->put( from.c_str(), from.size(), s(rec), rec.size() );
		acctStoreMap_.emplace( from, srcptr );
		i("I0023 added user account [%s]", s(from) );
	} else {
		i("E50387 error from=[%s] acct already exist", s(from) );
		return -10;
	}

	return 0;
}

int BlockMgr::updateAcctBalances( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	sstr to = trxn.receiver_;
	d("a32037 updateAcctBalances trxnId=[%s] from=[%s] to=[%s]", s(trxnId), s(from), s(to) );

	OmstorePtr srcptr;
	OmstorePtr dstptr;
	sstr fpath;
	sstr ts; trxn.getTrxnData(ts);
	double amt = trxn.getAmountDouble();

	//int  fromstat = 0;
	//int  tostat = 0;

	auto itr1 = acctStoreMap_.find( from );
	if ( itr1 == acctStoreMap_.end() ) {
		/***
		fpath = getAcctStoreFilePath( from );
		srcptr = new OmStore( fpath.c_str(), OM_DB_WRITE );
		fromstat = 1;
		**/
		i("E22276 error user from=[%s] does not exist", s(from));
		return -90;
	} else {
		srcptr = itr1->second;
		//fromstat = 2;
	}

	auto itr2 = acctStoreMap_.find( to );
	if ( itr2 == acctStoreMap_.end() ) {
		/**
		fpath = getAcctStoreFilePath( to );
		dstptr = new OmStore( fpath.c_str(), OM_DB_WRITE );
		tostat = 1;
		***/
		i("E23276 error user to=[%s] does not exist", s(to));
		return -92;
	} else {
		dstptr = itr2->second;
		//tostat = 2;
	}

	char *fromrec = srcptr->get( from.c_str() );
	char *torec = dstptr->get( to.c_str() );

	if ( NULL != fromrec && NULL != torec ) {
		/***
		frombal = atof(fromrec);
		tobal = atof(torec);
		frombal -= amt;
		tobal += amt;

		sprintf(abuf, "%.6f", frombal);
		srcptr->put( from.c_str(), from.size(), abuf, strlen(abuf) ); 

		sprintf(abuf, "%.6f", tobal);
		dstptr->put( to.c_str(), to.size(), abuf, strlen(abuf) ); 
		***/
		OmAccount fromAcct( fromrec );
		OmAccount toAcct( torec );
		double bal = fromAcct.addBalance( 0.0 - amt);
		if ( bal < -1.0 ) {
			i("E40313 error from=[%s] acct balance error [%f]", s(from), bal );
			return -30;
		}

		toAcct.addBalance( amt );

		sstr fromNew, toNew;
		fromAcct.str( fromNew );
		toAcct.str( toNew );

		srcptr->put( from.c_str(), from.size(), fromNew.c_str(), fromNew.size() );
		dstptr->put( to.c_str(), to.size(), toNew.c_str(), toNew.size() );

		/***
		if ( 1 == fromstat ) {
			// add store pointer for the from user
			acctStoreMap_.emplace( from, srcptr );
			d("98272 from, srcptr added");
		}

		if ( 1 == tostat ) {
			// add store pointer for the to user
			acctStoreMap_.emplace( to, dstptr );
			d("98272 to, dstptr added");
		}
		***/

	} else {
		if ( NULL == fromrec ) {
			i("E40387 error from=[%s] acct does not exist", s(from) );
			return -1;
		}
		if ( NULL == torec ) {
			i("E40388 error to=[%s] acct does not exist", s(to) );
			return -2;
		}
	}

	return 0;
}

#if 0
void BlockMgr::queryTrxn( const sstr &from, const sstr &to, const sstr &trxnId, const sstr &timestamp, sstr &res )
{
	// find in mempool first
	sstr tid;
	for ( auto& t: mempool_ ) {
		t.getTrxnID( tid );
		if ( tid == trxnId ) {
			t.getTrxnData( res );
			return;
		}
	}

	auto itr = trxnStoreMap_.find( trxnId );
	if ( itr == trxnStoreMap_.end() ) {
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
#endif

void BlockMgr::queryTrxn( const sstr &from, const sstr &trxnId, const sstr &timestamp, sstr &res )
{
	std::vector<sstr> vec;
	char tstat = '0';
	sstr err;
	int rc = readTrxns( from, timestamp, trxnId, vec, tstat, err );

	if ( rc < 0 || tstat == 'F' ) {
		if ( rc < 0 ) {
			res = trxnId + "|FAILED|" + err;
		} else {
			res = trxnId + "|FAILED|TrxnError";
		}
		i("E30298 %s", err.c_str() );
		return;
	}

	if ( vec.size() < 1 ) {
		//may be OK, just late
		res = trxnId + "|NOTFOUND";
		return;
	}

	res = vec[0];

}

// get a list of trxns of user from. If trxnId is not empty, get specific trxn
int BlockMgr::readTrxns(const sstr &from, const sstr &timestamp, const sstr &trxnId, std::vector<sstr> &vec, char &tstat, sstr &err )
{
	//d("a53001 readTrxns from=[%s] timestamp=[%s] trxnId=[%s]", s(from), s(timestamp), s(trxnId) );

	sstr yyyymmddhh = getYYYYMMDDHHFromTS(timestamp);

	sstr dir = dataDir_ + "/blocks/" + getUserPath(from) + "/" +  yyyymmddhh;
	sstr fpath = dir + "/blocks.blk";
	int fd = open( fpath.c_str(), O_RDONLY|O_NOATIME );
	if ( fd < 0 ) {
		i("E45508 error open from=[%s] [%s]", s(from), s(fpath) );
		err = "System error: unable to find user data";
		return -100;
	}
	lseek(fd, 0, SEEK_SET );
	//d("a2329 readTrxns from=[%s] fpath=[%s] offset=%ld", s(from), s(fpath), off);

	//fprintf(fp, "%ld~%c~%s~%ld~T~", tsize, ttype, tdata.c_str(), tsize );
	char c;
	int idx;
	char dbuf[16];
	int tsize, rdsize;
	char *pt = NULL;
	int rd;
	sstr trxn_tid;

	while ( true ) {
		// read trxnlength
		idx = 0;
		memset(dbuf, 0, 16);
		while ( 1==read(fd, &c, 1) ) {
			if ( idx > 7 ) {
				i("E12104 length field too long idx=%d dbuf=[%s]", idx, dbuf);
				err = "System error: E12104";
				::close(fd);
				return -1;
			}
			if ( ::isdigit(c) ) {
				dbuf[idx] = c;
				++idx;
				//d("a13104 idx=%d dbuf=[%s] c=[%c]", idx, dbuf, c);
			} else {
				break;
			}
		}
		if ( idx < 1 ) {
			::close(fd);
			//d("a33381 end of file");
			break;
		}
		dbuf[idx] = '\0';
		tsize = atoi(dbuf);
		//d("a11100 first dbuf=[%s] idx=%d", dbuf, idx );

		// fprintf(fp, "%ld~%c~%s~%ld~F~", tsize, ttype, tdata.c_str(), tsize );
		// c is '~' now
		rd = read(fd, &c, 1); // read in - or +
		if ( rd != 1 ) {
			i("E12214 ts read -/+ error" );
			err = "System error: E12214";
			::close(fd);
			return -10;
		}

		rd = read(fd, &c, 1); // read in ~
		if ( rd != 1 || c != '~' ) {
			i("E12215 ts read ~ error" );
			err = "System error: E12215";
			::close(fd);
			return -20;
		}

		// read in trxn data
		pt = (char*)malloc(tsize+1);
		pt[tsize] = '\0';
		rdsize = saferead(fd, pt, tsize);
		if ( rdsize != tsize ) {
			i("E12114 ts read size mismtach tsize=%d != rdsize=%d", tsize, rdsize);
			err = "System error: E12114";
			::close(fd);
			free(pt);
			return -1;
		}
		//d("a10234 tsize=%d", tsize );

		rd = read(fd, &c, 1); // read in ~
		if ( rd != 1 || c != '~' ) {
			i("E12216 ts read ~ error" );
			err = "System error: E12216";
			::close(fd);
			free(pt);
			return -30;
		}

		// read trxnlength again
		idx = 0;
		memset(dbuf, 0, 16);
		while ( 1==read(fd, &c, 1) ) {
			if ( idx > 7 ) {
				i("E12113 second length field too long");
				err = "System error: E12113";
				::close(fd);
				free(pt);
				return -40;
			}
			if ( isdigit(c) ) {
				dbuf[idx] = c;
				++idx;
				//d("a21087 second dbuf=[%s] idx=%d", dbuf, idx );
			} else {
				break;
			}
		}
		if ( idx < 1 ) {
			i("E12115 ts read size error" );
			err = "System error: E12115";
			::close(fd);
			free(pt);
			return -50;
		}
		dbuf[idx] = '\0';
		rdsize = atoi(dbuf);
		if ( rdsize != tsize ) {
			i("E12116 warning second tsize=%d != rdsize=%d", tsize, rdsize);
		}
		//d("a11102 second dbuf=[%s] idx=%d", dbuf, idx );

		// c is '~' now
		rd = read(fd, &c, 1); // read in F/T
		if ( rd != 1 ) {
			i("E12215 ts read F/T error" );
			err = "System error: E12215";
			::close(fd);
			free(pt);
			return -60;
		}
		tstat = c;

		if ( c != 'F' && c != 'T' ) {
			i("E12245 ts read F/T error c=[%c]", c );
			err = "System error: E12245";
			::close(fd);
			free(pt);
			return -64;
		}

		rd = read(fd, &c, 1); // read in ~
		if ( rd != 1 || c != '~' ) {
			i("E12216 ts read ~ error" );
			err = "System error: E12216";
			::close(fd);
			free(pt);
			return -70;
		}

		// got trxndata
		OmStrSplit sp( pt, '|');
		if ( sp.length() < 10 ) {
			i("E12218 ts format error len=%d", sp.length() );
			err = "System error: E12218";
			::close(fd);
			free(pt);
			return -70;
		}

		if ( trxnId.size() > 0 ) {
			//trxn_sender = sp[4];
			//trxn_timestamp = sp[7];
			trxn_tid = sp[7] + ":" + sp[4];
			if ( trxn_tid == trxnId ) {
				//d("a5023 found trxn for tid=[%s]", s(trxnId) );
				vec.push_back(pt);
			}
		} 

		if ( pt ) {
			free(pt);
		}
	}

	::close(fd);
	//d("a56411 readTrxns from [%s] vec.size=%d", fpath.c_str(), vec.size() );
	/***
	if ( vec.size() > 0 ) {
		d("a19721 vec[0]=[%s]", s(vec[0]) );
	}
	***/
	return 0;
}

void BlockMgr::initDirs()
{
	makedirPath( dataDir_ );
	d("a44427 mkdir [%s]", dataDir_.c_str() );
	/***
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
	***/
}

sstr BlockMgr::getUserPath( const sstr &userid )
{
	XXH64_hash_t hash1 = XXH64( userid.c_str(), userid.size(), DIR_HASH_SEED1 ) % DIR_LEVEL1_NUM;
	XXH64_hash_t hash2 = XXH64( userid.c_str(), userid.size(), DIR_HASH_SEED2 ) % DIR_LEVEL2_NUM;
	sstr path = std::to_string(hash1) + "/" + std::to_string(hash2);
	return path;
}

sstr BlockMgr::getAcctStoreFilePath( const sstr &userid )
{
	sstr dir = dataDir_ + "/account/" + getUserPath(userid);
	makedirPath(dir);
	sstr fpath = dir + "/acctstore.hdb";
	return fpath;
}

// ttype: -: debit/payout   +: credit/receive
FILE *BlockMgr::appendToBlockchain( OmicroTrxn &t, const sstr &userid, char ttype, const sstr &yyyymmddhh )
{
	sstr dir = dataDir_ + "/blocks/" + getUserPath(userid) + "/" +  yyyymmddhh;
	makedirPath( dir );
	sstr fpath = dir + "/blocks.blk";
	FILE *fp = fopen(fpath.c_str(), "a");
	if ( ! fp ) {
		i("E45508 appendToBlockchain error open [%s]", s(fpath) );
		return NULL;
	}

	sstr tdata; t.getTrxnData( tdata );
	long tsize = tdata.size();
	// todo compress tdata, tsize would be compressed size
	fprintf(fp, "%ld~%c~%s~%ld~", tsize, ttype, tdata.c_str(), tsize );
	d("a56881 appended trxn to [%s]", fpath.c_str() );
	return fp;
}

void BlockMgr::rollbackFromBlockchain( OmicroTrxn &t, const sstr &userid, const sstr &yyyymmddhh )
{
	sstr dir = dataDir_ + "/blocks/" + getUserPath(userid) + "/" +  yyyymmddhh;
	sstr fpath = dir + "/blocks.blk";
	/***
	FILE *fp = fopen(fpath.c_str(), "a");
	if ( ! fp ) {
		i("E45508 error open [%s]", s(fpath) );
		return -1;
	}

	long tsize;
	sstr tdata; t.getTrxnData( tdata );
	tsize = tdata.size();
	// todo compress tdata, tsize would be compressed size
	fprintf(fp, "%ld~%c%s~%ld", tsize, ttype, tdata.c_str(), tsize );
	fclose(fp);
	d("a52881 wrote blocks to [%s]", fpath.c_str() );
	***/
}

double BlockMgr::getBalance( const sstr &from ) const
{
	OmstorePtr srcptr;

	auto itr = acctStoreMap_.find( from );
	if ( itr == acctStoreMap_.end() ) {
		return -999.0;
	}

	srcptr = itr->second;

	char *fromrec = srcptr->get( from.c_str() );
	if ( NULL == fromrec ) {
		return -9999.0;
	}

	OmAccount fromAcct( fromrec );
	return fromAcct.getBalance();
}


