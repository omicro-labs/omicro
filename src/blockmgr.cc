/*
 * Copyright (C) Omicro Authors
 *
 * Omicro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Omicro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the LICENSE file. If not, see <http://www.gnu.org/licenses/>.
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "xxHash/xxhash.h"
#include "blockmgr.h"
#include "omutil.h"
#include "omstrsplit.h"
#include "omaccount.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include "omresponse.h"
#include "omdom.h"
#include "omtoken.h"

EXTERN_LOGGING


BlockMgr::BlockMgr()
{
}

void BlockMgr::setDataDir(  const sstr &dataDir )
{
	dataDir_ = dataDir;
	initDirs();
	d("a4447 initDirs done");
	trxnList_.setDataDir( dataDir_ );
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

	FILE *fp2 = NULL; 
	if ( trxn.trxntype_ == OM_PAYMENT ) {
		fp2 = appendToBlockchain(trxn, trxn.receiver_, '+', yyyymmddhh);
		if ( NULL == fp2 ) {
			fclose( fp1 );
			rollbackFromBlockchain(trxn, trxn.sender_, yyyymmddhh );
			return -2;
		}
	}

	int urc;
	if ( trxn.trxntype_ == OM_PAYMENT ) {
		urc = updateAcctBalances(trxn);
	} else if ( trxn.trxntype_ == OM_NEWACCT ) {
		urc = createAcct(trxn);
	} else if ( trxn.trxntype_ == OM_NEWTOKEN ) {
		urc = createToken(trxn);
	} else {
		urc = -10;
	}

	d("a23021 urc=%d 0 is OK", urc );

	if ( urc < 0 ) {
		// mark log failure 'F'
		fprintf(fp1, "F}");
		if ( fp2 ) {
			fprintf(fp2, "F}");
		}
	} else {
		// mark log success 'T'
		fprintf(fp1, "T}");
		if ( fp2 ) {
			fprintf(fp2, "T}");
		}
		d("a123001 success 'T' saveTrxnList ...");
		trxnList_.saveTrxnList( trxn.sender_, trxn.timestamp_ );
	}

	fclose(fp1);
	if ( fp2 ) {
		fclose(fp2);
	}

	return 0;
}

int BlockMgr::createAcct( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	d("a32047 createAcct trxnId=[%s] from=[%s]", s(trxnId), s(from) );

	OmstorePtr srcptr;
	sstr fpath;
	// sstr ts; trxn.getTrxnData(ts);

	auto itr1 = acctStoreMap_.find( from );
	if ( itr1 == acctStoreMap_.end() ) {
		fpath = getAcctStoreFilePath( from );
		srcptr = new OmStore( fpath.c_str(), OM_DB_WRITE );

		OmDom dom( trxn.request_ );
		sstr accttype;
		dom.get("ACTYPE", accttype );
		if ( accttype != OM_ACCT_USER && accttype != OM_ACCT_CONTRACT ) {
			i("E50284 error newacct from=[%s] accttype=[%s] invalid", s(from), s(accttype) );
			delete srcptr;
			return -5;
		}

		OmAccount acct;
		acct.accttype_ = accttype;
		acct.balance_ = "10000000";
		acct.tokentype_ = "O";   // omicro
		acct.pubkey_ = trxn.userPubkey_;
		acct.keytype_ = "DL5";
		acct.out_ = "0";
		acct.in_ = "0";

		sstr rec;
		acct.json( rec );

		srcptr->put( from.c_str(), from.size(), s(rec), rec.size() );
		acctStoreMap_.emplace( from, srcptr );
		i("I0023 added user account [%s]", s(from) );
	} else {
		i("E50387 error from=[%s] acct already exist", s(from) );
		return -10;
	}

	return 0;
}

// a user create some tokens
int BlockMgr::createToken( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	d("a32047 createAcct trxnId=[%s] from=[%s]", s(trxnId), s(from) );

	//each token under owner has its own contractId/address
	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E30821 createToken error from=[%s] not created yet.", s(from));
		return -10;
	}

	OmAccount acct(fromrec);

	if ( acct.tokens_.size() < 1 ) {
		acct.tokens_ = trxn.request_;
	} else {
		// make sure trxn.request_ has no identical names in acct.tokens_
		bool dup = OmToken::hasDupNames( acct.tokens_, trxn.request_);
		if ( dup ) {
			i("E30220 error from=[%s] has dup names", s(from));
			i("E30220 error trxn.request_=[%s]", s(trxn.request_));
			i("E30220 error acct.tokens_=[%s]", s(acct.tokens_) );
			return -15;
		}

		// qwer
		std::string &s = acct.tokens_;
		s.erase(s.find_last_not_of("]")+1);
		// trim right ]


		//trim left of trxn.request_
		const char *p = trxn.request_.c_str();
		while ( *p != '[' && *p != '\0' ) ++p; 
		if ( *p == '[' ) {
			++p; // skip [
		} else {
			//p = trxn.request_.c_str();
			return -20;
		}

		acct.tokens_ = acct.tokens_ + "," + std::string(p);
	}

	sstr newjson;
	acct.json( newjson );

	srcptr->put( from.c_str(), from.size(), newjson.c_str(), newjson.size() );
	i("I2023 user=[%s]  added tokens [%s]", s(from), s(trxn.request_) );
	i("I2023 user=[%s]  all tokens [%s]", s(from), s(acct.tokens_) );

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

	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E22276 error user from=[%s] does not exist", s(from));
		return -90;
	}

	char *torec = findSaveStore( to, dstptr );
	if ( ! torec ) {
		i("E23276 error user to=[%s] does not exist", s(to));
		return -92;
	}

	OmAccount fromAcct( fromrec );
	OmAccount toAcct( torec );
	double bal = fromAcct.addBalance( 0.0 - amt);
	if ( bal < -1.0 ) {
		i("E40313 error from=[%s] acct balance error [%f]", s(from), bal );
		return -30;
	}
	
	fromAcct.incrementFence();

	toAcct.addBalance( amt );
	toAcct.incrementIn();

	sstr fromNew, toNew;
	fromAcct.json( fromNew );
	toAcct.json( toNew );

	srcptr->put( from.c_str(), from.size(), fromNew.c_str(), fromNew.size() );

	dstptr->put( to.c_str(), to.size(), toNew.c_str(), toNew.size() );

	return 0;
}

void BlockMgr::queryTrxn( const sstr &from, const sstr &trxnId, const sstr &timestamp, sstr &res )
{
	std::vector<sstr> vec;
	char tstat = '0';
	sstr err;
	int rc = readTrxns( from, timestamp, trxnId, vec, tstat, err );
	d("a56702 queryTrxn readTrxns rc=%d from=[%s] trxnId=[%s] timestamp=[%s]", rc, s(from), s(trxnId), s(timestamp) );


	OmResponse resp;
	resp.TID_ = trxnId;
	resp.UID_ = from;

	
	if ( rc < 0 || tstat == 'F' ) {
		resp.STT_ = OM_RESP_ERR;
		resp.RSN_ = "FAILED";
		if ( rc < 0 ) {
			resp.DAT_ = err;
		} else {
			resp.DAT_ = "TRXNERROR";
		}
		resp.json( res );
		i("E30298 %s", err.c_str() );
		return;
	}

	if ( vec.size() < 1 ) {
		//may be OK, just late
		resp.STT_ = OM_RESP_ERR;
		resp.RSN_ = "NOTFOUND";
		resp.json( res );
		return;
	}

	OmicroTrxn t( vec[0].c_str() );
	resp.REQ_ = t.request_;

	resp.STT_ = OM_RESP_OK;
	resp.json( res );
}

// get a list of trxns of user from. If trxnId is not empty, get specific trxn
int BlockMgr::readTrxns(const sstr &from, const sstr &timestamp, const sstr &trxnId, std::vector<sstr> &vec, char &tstat, sstr &err )
{
	d("a53001 readTrxns from=[%s] timestamp=[%s] trxnId=[%s]", s(from), s(timestamp), s(trxnId) );

	sstr yyyymmddhh = getYYYYMMDDHHFromTS(timestamp);

	sstr dir = dataDir_ + "/blocks/" + getUserPath(from) + "/" +  yyyymmddhh;
	sstr fpath = dir + "/blocks";
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
		if ( rd != 1 || c != '}' ) {
			i("E12216 ts read } error" );
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
	d("a32220 return 0 here");
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
	sstr fpath = dir + "/store.hdb";
	d("a42061 getAcctStoreFilePath userid=[%s] fpath=[%s]", s(userid), s(fpath) );
	return fpath;
}

// ttype: -: debit/payout   +: credit/receive
FILE *BlockMgr::appendToBlockchain( OmicroTrxn &t, const sstr &userid, char ttype, const sstr &yyyymmddhh )
{
	d("a10072 appendToBlockchain userid=[%s] ttype=[%c] yyyymmddhh=[%s]", s(userid), ttype, s(yyyymmddhh) );
	sstr dir = dataDir_ + "/blocks/" + getUserPath(userid) + "/" +  yyyymmddhh;
	makedirPath( dir );
	sstr fpath = dir + "/blocks";
	FILE *fp = fopen(fpath.c_str(), "a");
	if ( ! fp ) {
		i("E45508 appendToBlockchain error open [%s]", s(fpath) );
		return NULL;
	}

	sstr tdata; t.getTrxnData( tdata );
	long tsize = tdata.size();
	// todo compress tdata, tsize would be compressed size
	fprintf(fp, "%ld~%c~%s~%ld~", tsize, ttype, tdata.c_str(), tsize );
	d("a56881 userid=[%s] appended trxn to fpath [%s]", s(userid),  fpath.c_str() );
	return fp;
}

void BlockMgr::rollbackFromBlockchain( OmicroTrxn &t, const sstr &userid, const sstr &yyyymmddhh )
{
	sstr dir = dataDir_ + "/blocks/" + getUserPath(userid) + "/" +  yyyymmddhh;
	sstr fpath = dir + "/blocks";
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

double BlockMgr::getBalance( const sstr &from ) 
{
	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E22276 error user from=[%s] does not exist", s(from));
		return -90;
	}

	OmAccount fromAcct( fromrec );
	return fromAcct.getBalance();
}

void BlockMgr::getTokens( const sstr &from, sstr &tokens ) 
{
	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E21776 error user from=[%s] does not exist", s(from));
		return;
	}

	OmAccount fromAcct( fromrec );
	tokens = fromAcct.tokens_;
}

int BlockMgr::getBalanceAndPubkey( const sstr &from, double &bal, sstr &pubkey )
{
	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E22216 error user from=[%s] does not exist", s(from));
		return -90;
	}

	OmAccount fromAcct( fromrec );
	bal = fromAcct.getBalance();
	pubkey = fromAcct.pubkey_;
	return 0;
}

void BlockMgr::getFence( const sstr &from, sstr &fence )
{
	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E21216 error user from=[%s] does not exist", s(from));
		return;
	}

	OmAccount fromAcct( fromrec );
	fence = fromAcct.out_;
}

int BlockMgr::runQuery( OmicroTrxn &trxn, sstr &res )
{
	sstr from = trxn.sender_;

	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E15216 error user from=[%s] does not exist", s(from));
		return -90;
	}

	rapidjson::Document dom;
	dom.Parse( trxn.request_.c_str() );
	if ( dom.HasParseError() ) {
		d("a30280 from=[%s] dom.HasParseError", s(from) );
        return -30;
    }

	//const rapidjson::Value& pred = dom["predicate"];
	const rapidjson::Value& fields = dom["fields"];
	if ( ! fields.IsArray() ) {
		d("a30280 from=[%s] fields not array", s(from) );
        return -40;
	}

	OmAccount fromAcct( fromrec );
	const char *p;
	rapidjson::StringBuffer sbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
	writer.StartObject();

	for ( rapidjson::SizeType  i = 0; i < fields.Size(); i++) {
		const rapidjson::Value &v = fields[i];
		if ( ! v.IsString() ) {
			continue;
		}

		p = v.GetString();
		if ( 0 == strcmp(p, "balance") ) {
			writer.Key(p);
			writer.String(fromAcct.balance_.c_str());
		} else if ( 0 == strcmp(p, "out") ) {
			writer.Key(p);
			writer.String(fromAcct.out_.c_str());
		} else if ( 0 == strcmp(p, "in") ) {
			writer.Key(p);
			writer.String(fromAcct.in_.c_str());
		} else if ( 0 == strcmp(p, "tokentype") ) {
			writer.Key(p);
			writer.String(fromAcct.tokentype_.c_str());
		} else if ( 0 == strcmp(p, "accttype") ) {
			writer.Key(p);
			writer.String(fromAcct.accttype_.c_str());
		} else if ( 0 == strcmp(p, "tokens") ) {
			writer.Key(p);
			writer.String(fromAcct.tokens_.c_str());
		}
	}

	writer.EndObject();
	res =  sbuf.GetString();
	return 0;
}

// FInd and save user store if users exists. If not exists, return false
char *BlockMgr::findSaveStore( const sstr &userId, OmstorePtr &ptr )
{
	sstr fpath;
	auto itr = acctStoreMap_.find( userId );
	if ( itr == acctStoreMap_.end() ) {
		fpath = getAcctStoreFilePath( userId );
		ptr = new OmStore( fpath.c_str(), OM_DB_WRITE );
		char *rec = ptr->get( userId.c_str() );
		if ( rec ) {
			acctStoreMap_.emplace( userId, ptr );
			return rec; 
		} else {
			delete ptr;
			ptr = NULL;
			return NULL; 
		}
	} else {
		ptr = itr->second;
		char *rec = ptr->get( userId.c_str() );
		if ( rec ) {
			return rec; 
		} else {
			return NULL; 
		}
	}
}

void BlockMgr::saveTrxnList( const sstr &from, const sstr &timestamp )
{
	trxnList_.saveTrxnList( from, timestamp );
}


