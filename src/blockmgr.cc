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
#define  RAPIDJSON_HAS_STDSTRING 1
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
#include "omlimits.h"
#include "omjson.h"
EXTERN_LOGGING

/**********************************************************************
**
**  API to storage and blockchain
**
**  It handles mempool and permanent storage. Also it checks conditions
**  of accounts, tokens, and balances.
**
**
**********************************************************************/
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
	/*** todo
	memPool_.push_back( trxn );
	if ( memPool_.size() > OM_MEMPOOL_SZ ) {
		i("I10026  memPool_ flush ... ");
		for ( auto &t : memPool_ ) {
			saveTrxn( t );
		}
		memPool_.clear();
		i("I10028  memPool_ cleared");
	}
	return 0;
	***/
	return saveTrxn( trxn );
}

int BlockMgr::saveTrxn( OmicroTrxn &trxn)
{
	sstr yyyymmddhh = getYYYYMMDDHHFromTS(trxn.timestamp_);
	d("a65701 receiveTrxn ...");

	int urc;
	if ( trxn.trxntype_ == OM_PAYMENT ) {
		urc = updateAcctBalances(trxn);
	} else if ( trxn.trxntype_ == OM_NEWACCT ) {
		urc = createAcct(trxn);
	} else if ( trxn.trxntype_ == OM_NEWTOKEN ) {
		urc = createToken(trxn);
	} else if ( trxn.trxntype_ == OM_XFERTOKEN ) {
		d("a38103 transferToken ...");
		urc = transferToken(trxn);
	} else {
		urc = -10;
	}

	d("a23021 urc=%d 0 is OK", urc );

	if ( urc < 0 ) {
		d("a444401 urc < 0 error");
	} else {
    	long pos1;
    	FILE *fp1 = appendToBlockchain(trxn, trxn.sender_, '-', yyyymmddhh, pos1);
    	if ( NULL == fp1 ) {
    		return -1;
    	}
    
    	FILE *fp2 = NULL; 
    	if ( trxn.trxntype_ == OM_PAYMENT || trxn.trxntype_ == OM_XFERTOKEN ) {
    		long pos2;
    		fp2 = appendToBlockchain(trxn, trxn.receiver_, '+', yyyymmddhh, pos2);
    		if ( NULL == fp2 ) {
    			fclose( fp1 );
    			rollbackFromBlockchain(trxn, trxn.sender_, yyyymmddhh, pos1 );
    			return -2;
    		}
    	}

		// mark log success 'T'
		fprintf(fp1, "T}");
		if ( fp2 ) {
			fprintf(fp2, "T}");
		}

		fclose(fp1);
		if ( fp2 ) {
			fclose(fp2);
		}

		d("a123001 success 'T' saveTrxnList ...");
		trxnList_.saveTrxnList( trxn.sender_, trxn.timestamp_ );
	}

	return 0;
}

// Create a new account
int BlockMgr::createAcct( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	d("a32047 createAcct trxnId=[%s] from=[%s]", s(trxnId), s(from) );

	if ( from.size() > OM_NAME_MAXSZ ) {
		i("E40214 newacct from=[%s] too long", s(from) );
		return -1;
	}

	OmstorePtr srcptr;
	sstr fpath;

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
		i("I0023 added user account [%s] %s:%s", s(from), s(srv_), s(port_) );
	} else {
		i("E50387 error from=[%s] acct already exist", s(from) );
		return -10;
	}

	return 0;
}

// A user creates some tokens  FT or NFT or both
int BlockMgr::createToken( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	d("a32047 createToken trxnId=[%s] from=[%s]", s(trxnId), s(from) );

	//each token under owner has its own contractId/address
	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E30821 createToken error from=[%s] not created yet.", s(from));
		return -10;
	}

	// trxn.request_ "[{..}, {...}, {...} ]"
	int rc = validateReqTokens( from, trxn.request_ );
	if ( rc < 0 ) {
		i("E30824 createToken error from=[%s] trxn.request_=[%s] not valid.", s(from), s(trxn.request_) );
		return -10;
	}

	OmAccount acct(fromrec);

	if ( acct.tokens_.size() < 1 ) {
		acct.tokens_ = trxn.request_;
		d("a51280 nitial acct.tokens_=[%s]", s(acct.tokens_) );
	} else {
		// make sure trxn.request_ has no identical names in acct.tokens_
		bool dup = OmToken::hasDupNames( acct.tokens_, trxn.request_);
		if ( dup ) {
			i("E30220 error from=[%s] has dup token names", s(from));
			i("E30220 error trxn.request_=[%s]", s(trxn.request_));
			i("E30220 error acct.tokens_=[%s]", s(acct.tokens_) );
			return -15;
		}

		sstr &str = acct.tokens_;
		str.erase(str.find_last_not_of("]")+1);
		// trim right ]

		//trim left of trxn.request_
		const char *p = trxn.request_.c_str();
		while ( *p != '[' && *p != '\0' ) ++p; 
		if ( *p == '[' ) {
			++p; // skip [
		} else {
			return -20;
		}

		acct.tokens_ = acct.tokens_ + "," + sstr(p);
		d("a51281 new acct.tokens_=[%s]", s(acct.tokens_) );
	}

	sstr newjson;
	acct.json( newjson );

	srcptr->put( from.c_str(), from.size(), newjson.c_str(), newjson.size() );
	i("I2023 user=[%s]  added tokens [%s]", s(from), s(trxn.request_) );
	i("I2023 user=[%s]  all tokens [%s]", s(from), s(acct.tokens_) );

	return 0;
}

// Make a payment and update both balances (from and to)
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
	d("a333081 from=[%s] pay after incrementFence [%s] peer=[%s]", s(from), s(fromAcct.out_), s(trxn.srvport_) );

	toAcct.addBalance( amt );
	toAcct.incrementIn();

	sstr fromNew, toNew;
	fromAcct.json( fromNew );
	toAcct.json( toNew );

	srcptr->put( from.c_str(), from.size(), fromNew.c_str(), fromNew.size() );

	dstptr->put( to.c_str(), to.size(), toNew.c_str(), toNew.size() );

	return 0;
}

// Check status of a transaction
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
		if ( rc < 0 ) {
			if ( rc  == -1000 ) {
				resp.RSN_ = "NOTFOUND";
			} else {
				resp.RSN_ = "FAILED";
			}
			resp.DAT_ = err;
		} else {
			resp.RSN_ = "FAILED";
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

// Get a list of trxns of user from. If trxnId is not empty, get specific trxn
int BlockMgr::readTrxns(const sstr &from, const sstr &timestamp, const sstr &trxnId, std::vector<sstr> &vec, char &tstat, sstr &err )
{
	sstr yyyymmddhh = getYYYYMMDDHHFromTS(timestamp);

	sstr dir = dataDir_ + "/blocks/" + getUserPath(from) + "/" +  yyyymmddhh;
	sstr fpath = dir + "/blocks";
	d("a53001 readTrxns from=[%s] timestamp=[%s] trxnId=[%s] fpath=[%s]", s(from), s(timestamp), s(trxnId), s(fpath) );
	int fd = open( fpath.c_str(), O_RDONLY|O_NOATIME );
	if ( fd < 0 ) {
		i("E45508 error open from=[%s] [%s]", s(from), s(fpath) );
		err = sstr("System: cannot find user data [") + from + "] " +  srv_ + ":" + port_;
		return -1000;
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
		if ( idx < 1 || c == 0 ) {
			::close(fd);
			//d("a33381 end of file or see NULL byte");
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
				break; // got ~
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
			//trxn_sender = sp[2];
			//trxn_timestamp = sp[5];
			// refer trxn structure
			trxn_tid = sp[5] + ":" + sp[2];
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
	d("a32220 return 0 here vec.size=%d", vec.size() );
	return 0;
}

void BlockMgr::initDirs()
{
	makedirPath( dataDir_ );
	d("a44427 mkdir [%s]", dataDir_.c_str() );
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
FILE *BlockMgr::appendToBlockchain( OmicroTrxn &t, const sstr &userid, char ttype, const sstr &yyyymmddhh, long &fpos )
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

	fpos = ftell(fp);

	sstr tdata; t.getTrxnData( tdata );
	long tsize = tdata.size();
	// todo compress tdata, tsize would be compressed size
	fprintf(fp, "%ld~%c~%s~%ld~", tsize, ttype, tdata.c_str(), tsize );
	d("a56881 userid=[%s] appended trxn to fpath [%s]", s(userid),  fpath.c_str() );
	return fp;
}

void BlockMgr::rollbackFromBlockchain( OmicroTrxn &t, const sstr &userid, const sstr &yyyymmddhh, long fpos )
{
	sstr dir = dataDir_ + "/blocks/" + getUserPath(userid) + "/" +  yyyymmddhh;
	sstr fpath = dir + "/blocks";
	FILE *fp = fopen(fpath.c_str(), "w");
	if ( ! fp ) {
		i("E45608 error open [%s]", s(fpath) );
		return;
	}

	fseek(fp, fpos, SEEK_SET );
	fprintf(fp, "%c", 0 ); // NULL byte ending
	fclose(fp);
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
	//d("a62201 BlockMgr::getFence from=[%s] fromrec=[%s]", s(from), s(fromrec), s(fence) );
	//d("a62201 BlockMgr::getFence from=[%s] fence=out_=[%s]", s(from), s(fence) );
}

// Run a query
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
	dom.Parse( trxn.request_ );
	if ( dom.HasParseError() ) {
		d("a30280 from=[%s] dom.HasParseError", s(from) );
        return -30;
    }

	//const rapidjson::Value& pred = dom["predicate"];
	sstr predicate;
	auto itr = dom.FindMember("predicate");
	if ( itr != dom.MemberEnd() ) {
		predicate = itr->value.GetString();
	}
	d("a39338 runQuery predicate=[%s]", s(predicate) );

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

	for ( rapidjson::SizeType  idx = 0; idx < fields.Size(); ++idx ) {
		const rapidjson::Value &v = fields[idx];
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
			writer.String(fromAcct.tokentype_);
		} else if ( 0 == strcmp(p, "accttype") ) {
			writer.Key(p);
			writer.String(fromAcct.accttype_);
		} else if ( 0 == strcmp(p, "tokens") ) {
			writer.Key(p);
			writer.String(fromAcct.tokens_);
		} else if ( 0 == strcmp(p, "token") ) {
			if ( predicate.size() > 0 ) {
				// expect "name=mytokenname123"
				OmStrSplit sp(predicate, '=');
				if ( sp.length() == 2 && sp[0] == "name" ) {
					// find object where name==sp[1]
					sstr resultObj;
					OmJson::getObjStr( sp[1], fromAcct.tokens_, resultObj );
					d("a33408 resultObj=[%s]", s(resultObj) );
					if ( resultObj.size() > 0 ) {
						writer.Key("token");
						writer.String( resultObj );
					} else {
						i("E56231 not found fromAcct.tokens_=[%s]", s(fromAcct.tokens_) );
						writer.Key("token");
						writer.String( "" );
					}
				}
			}

			//writer.String(fromAcct.tokens_.c_str());
		}
	}

	writer.EndObject();
	res =  sbuf.GetString();
	return 0;
}

// Find and save user store if users exists. If not exists, return false
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

// Transfer tokens from one account to another
int BlockMgr::transferToken( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	sstr to = trxn.receiver_;
	d("a32007 transferToken trxnId=[%s] from=[%s] to=[%s]", s(trxnId), s(from), s(to) );

	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E30821 transferToken error from=[%s] not created yet.", s(from));
		return -10;
	}

	OmstorePtr dstptr;
	char *torec = findSaveStore( to, dstptr );
	if ( ! torec ) {
		i("E30321 transferToken error to=[%s] not created yet.", s(to));
		return -20;
	}

	OmAccount fromacct(fromrec);

	if ( fromacct.tokens_.size() < 1 ) {
		//acct.tokens_ = trxn.request_;
		i("E33401 from=[%s] tokens_ is empty", s(from) );
		return -30;
	}

	bool exist = OmToken::hasDupNames( fromacct.tokens_, trxn.request_);
	if ( ! exist ) {
		i("E22303 from=[%s] tokens_ does not have the tokens to be xfered", s(from) );
		i("E22303 fromacct.tokens_=[%s]", s(fromacct.tokens_) );
		i("E22303 trxn.request_=[%s]", s(trxn.request_) );
		return -40;
	}

	OmAccount toacct(torec);

	// modify fromacct.tokens_  and toacct.tokens_
	int rc = modifyTokens(from, to, trxn.request_, fromacct.tokens_, toacct.tokens_ );
	if ( rc < 0 ) {
		i("E21303 from=[%s] to=[%s] modifyTokens error rc=%d", s(from), s(to), rc );
		i("E21303 fromacct.tokens_=[%s]", s(fromacct.tokens_) );
		i("E21303 trxn.request_=[%s]", s(trxn.request_) );
		return -50;
	}

	sstr fromnewjson;
	fromacct.incrementFence();
	d("a30042 from=[%s] tokenxfer incrementFence [%s] peer=[%s]", s(from), s(fromacct.out_), s(trxn.srvport_) );
	fromacct.json( fromnewjson );

	sstr tonewjson;
	toacct.incrementIn();
	toacct.json( tonewjson );

	srcptr->put( from.c_str(), from.size(), fromnewjson.c_str(), fromnewjson.size() );
	i("I2023 user=[%s]  xfered tokens [%s]", s(from), s(trxn.request_) );
	i("I2023 user=[%s]  all tokens [%s]", s(from), s(fromacct.tokens_) );
	i("I2023 user=[%s]  all tokens [%s]", s(to), s(toacct.tokens_) );

	dstptr->put( to.c_str(), to.size(), tonewjson.c_str(), tonewjson.size() );

	return 0;
}

// reqJson: [{ "name": "tok1", "amount": "123" }, {...}, {...}]  if no amount, default is 1
// fromTokens: [{ "name": "tok1", "max": "1239999", "bal": "2221", "url": "http://xxx", "other": "zzzzz" }, {...}, {...}]
// toTokens: "" empty or
// toTokens: [{ "name": "tok4", "max": "12999", "bal": "22", "url": "http://xxx", "some": "yyy" }, {...}, {...}]
int BlockMgr::modifyTokens( const sstr &from, const sstr &to,  const sstr &reqJson, sstr &fromTokens, sstr &toTokens )
{
	using namespace rapidjson;
	Document fromdom;
	fromdom.Parse( fromTokens );
	if ( fromdom.HasParseError() ) {
		i("E32231 modifyTokens fromTokens invalid");
		return -10;
	}

	Document todom;
	if ( toTokens.size() > 0 ) {
		todom.Parse( toTokens );
		if ( todom.HasParseError() ) {
			i("E32233 modifyTokens toTokens invalid");
			return -20;
		}
	} else {
		todom.Parse( "[]" );
	}

	Document chdom;
	chdom.Parse( reqJson );
	if ( chdom.HasParseError() ) {
		i("E32235 modifyTokens reqJson invalid");
		i("reqJson=%s", s(reqJson) );
		return -30;
	}

	if ( ! chdom.IsArray() ) {
		i("E33035 modifyTokens chdom is not array");
		return -40;
	}

	Value::ConstMemberIterator itr;
	double damt;
	char buf[32];
	sstr xferAmount;

	bool fromHasToken;
	bool toHasToken;
	bool isNFT;

   	for ( rapidjson::SizeType idx = 0; idx < chdom.Size(); ++idx) {
		const rapidjson::Value &rv = chdom[idx];
		if ( ! rv.IsObject() ) {
			i("E32005 not object");
			return -41;
		}

		// rv is object { "name": "tok1", "amount": "234" }
		itr = rv.FindMember("name");
		if ( itr == rv.MemberEnd() ) {
			i("E32006 no name");
			return -42;
		}
		const sstr &xferName = itr->value.GetString();
		if ( xferName.size() > OM_TOKEN_MAX_LEN ) {
			i("E32007 name too long");
			return -44;
		}

		itr = rv.FindMember("amount");
		if ( itr == rv.MemberEnd() ) {
			xferAmount = "1";
		} else {
    		if ( itr->value.IsString() ) {
    			//printf("[%s] amount a33933930 is string !!!!\n", s(name));
        		const sstr &amt = itr->value.GetString();
        		if ( amt.size() > OM_NUM_MAXSZ ) {
        			continue;
        		}
        		if ( atof( amt.c_str() ) <= 0 ) {
        			i("E37023 error amt=[%s]", s(amt) );
        			return -45;
        		}
        		xferAmount = amt;
    		} else {
        		i("E31043 error not string" );
        		return -46;
    		}
		}

		d("a2221 xferAmount=[%s]", s(xferAmount));
		if ( xferAmount.size() > OM_NUM_MAXSZ ) {
        	i("E31443 error xferAmount size too long" );
        	return -48;
		}

		// deduct from tokens
		fromHasToken = false;
		isNFT = false;
   		for ( rapidjson::SizeType j = 0; j < fromdom.Size(); ++j) {
			rapidjson::Value &v = fromdom[j];
			if ( ! v.IsObject() ) {
				i("E12287 not object");
				return -70;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				i("E12237 not name");
				return -70;
			}
			const sstr &tname = itr->value.GetString();
			if ( tname != xferName ) {
				continue;
			}

			itr = v.FindMember("max");
			if ( itr == v.MemberEnd() ) {
				i("E12230 not max");
				return -70;
			}
			const sstr &max = itr->value.GetString();
			if ( atoll(max.c_str() ) == 1 ) {
				itr = v.FindMember("owner");
				if ( itr == v.MemberEnd() ) {
					i("E32474 nft token no owner");
					return -81;
				}
				const sstr &towner = itr->value.GetString();
				if ( from != towner ) {
					i("E32431 nft token wrong owner from=[%s] towner=[%s]", s(from), s(towner));
					return -81;
				}
				// modify owner
				v["owner"].SetString( to, fromdom.GetAllocator() );
				isNFT = true;
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &bal = itr->value.GetString();

			damt = atof(bal.c_str()) - atof(xferAmount.c_str());
			if ( damt < 0.0 ) {
				i("E21722 error xferamount=[%s] iver bal=[%s]", xferAmount.c_str(), bal.c_str() );
				return -50;
			}

			sprintf(buf, "%.6g", damt );
			v["bal"].SetString( buf, fromdom.GetAllocator() );
			d("a22083 new from account xfername=[%s] bal_buf=[%s] origbal=[%s] xferAmount=[%s]", s(xferName), buf, s(bal), s(xferAmount) );
			fromHasToken = true;
			break;
		}

		if ( ! fromHasToken ) {
			//continue;  // do not touch todom
			i("E45023 fromacct has no such token [%s]", s(xferName) );
			return -55;
		}

		// check existing tokens in to account
		toHasToken = false;
   		for ( rapidjson::SizeType j = 0; j < todom.Size(); j++) {
			rapidjson::Value &v = todom[j];
			if ( ! v.IsObject() ) {
				continue;
			}

			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				continue;
			}

			const sstr &tname = itr->value.GetString();
			if ( tname != xferName ) {
				continue;
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}

			toHasToken = true;

			if ( isNFT ) {
				v["bal"].SetString( "1", todom.GetAllocator() );
				v["owner"].SetString( to, todom.GetAllocator() );
			} else {
				const sstr &bal = itr->value.GetString();
				damt = atof(bal.c_str()) + atof(xferAmount.c_str());
				sprintf(buf, "%.6g", damt );
				v["bal"].SetString( buf, todom.GetAllocator() );
				d("a222001 toaccount bal xfername=[%s] to=[%s] bal=[%s] xferAmoun=[%s]", s(xferName), buf, s(bal), s(xferAmount) );
			}

			break;
		}

		d("a93938 toHasToken=%d", toHasToken );

		if ( ! toHasToken ) {
			// find the oject with the mathching name in from
   			for ( rapidjson::SizeType j = 0; j < fromdom.Size(); ++j) {
				const rapidjson::Value &v = fromdom[j];
				if ( ! v.IsObject() ) {
					i("E23371 no object");
					return -90;
				}
				itr = v.FindMember("name");
				if ( itr == v.MemberEnd() ) {
					i("E23351 no name");
					return -90;
				}
				const sstr &tname = itr->value.GetString();
				if ( tname != xferName ) {
					continue;
				}

				// v is the keys-values object
				Value obj(kObjectType);
				obj.SetObject();
				int cnt = 0;
				for (auto& kv : v.GetObject()) {
					if ( 0 == strcmp(kv.name.GetString(), "bal" ) ) {
						obj.AddMember( StringRef(kv.name.GetString()), xferAmount, todom.GetAllocator() );
					} else {
						obj.AddMember( StringRef(kv.name.GetString()), StringRef(kv.value.GetString()), todom.GetAllocator() );
						// fromdom has changed owner for nft token
					}
					++cnt;
				}
				if ( cnt > 0 ) {
					todom.PushBack( obj, todom.GetAllocator() ); 
				}
				break;
			}
		}

		d("a22222\n");
	}

	StringBuffer sbuf;
	Writer<StringBuffer> writer(sbuf);
	fromdom.Accept(writer);
	fromTokens = sbuf.GetString();

	StringBuffer sbuf2;
	Writer<StringBuffer> writer2(sbuf2);
	todom.Accept(writer2);
	toTokens = sbuf2.GetString();

	//d("a23017 fromTokens=[%s]", s(fromTokens) );
	//d("a23017 toTokens=[%s]", s(toTokens) );
	return 0;
}

// reqJson: [{ "name": "tok1", "amount": "123" }, {...}, {...}]  if no amount, default is 1
// fromTokens: [{ "name": "tok1", "max": "1239999", "bal": "2221", "url": "http://xxx", "other": "zzzzz" }, {...}, {...}]
// toTokens: "" empty or
// toTokens: [{ "name": "tok4", "max": "12999", "bal": "22", "url": "http://xxx", "some": "yyy" }, {...}, {...}]
int BlockMgr::checkValidTokens( const sstr &from, const sstr &to, const sstr &reqJson, sstr &fromTokens, sstr &toTokens )
{
	using namespace rapidjson;
	Document fromdom;
	fromdom.Parse( fromTokens );
	if ( fromdom.HasParseError() ) {
		i("E34231 modifyTokens fromTokens invalid");
		return -10;
	}

	Document todom;
	if ( toTokens.size() > 0 ) {
		todom.Parse( toTokens );
		if ( todom.HasParseError() ) {
			i("E34233 modifyTokens toTokens invalid");
			return -20;
		}
	} 

	Document chdom;
	chdom.Parse( reqJson );
	if ( chdom.HasParseError() ) {
		i("E34235 modifyTokens reqJson invalid");
		i("reqJson=%s", s(reqJson) );
		return -30;
	}

	if ( ! chdom.IsArray() ) {
		i("E34035 modifyTokens chdom is not array");
		return -40;
	}

	Value::ConstMemberIterator itr;
	sstr xferAmount;
	bool fromHasToken;

	// check each tolen to be transfered
   	for ( rapidjson::SizeType idx = 0; idx < chdom.Size(); ++idx) {
		const rapidjson::Value &rv = chdom[idx];
		if ( ! rv.IsObject() ) {
        	i("E34128 error not object" );
        	return -35;
		}

		// rv is object { "name": "tok1", "amount": "234" }
		itr = rv.FindMember("name");
		if ( itr == rv.MemberEnd() ) {
        	i("E34020 error no name" );
        	return -45;
		}
		const sstr &xferName = itr->value.GetString();
		if ( xferName.size() > OM_TOKEN_MAX_LEN ) {
        	i("E34021 error name too long" );
        	return -46;
		}

		itr = rv.FindMember("amount");
		if ( itr == rv.MemberEnd() ) {
			xferAmount = "1";
		} else {
    		if ( itr->value.IsString() ) {
        		const sstr &amt = itr->value.GetString();
        		if ( amt.size() > OM_NUM_MAXSZ ) {
					i("E34022 error amt too big" );
					return -57;
        		}
        		if ( atof( amt.c_str() ) <= 0 ) {
        			i("E34023 error amt=[%s]", s(amt) );
        			return -45;
        		}
        		xferAmount = amt;
    		} else {
        		i("E34043 error not string" );
        		return -46;
    		}
		}

		if ( xferAmount.size() > OM_NUM_MAXSZ ) {
        	i("E35043 error xferAmount too long" );
        	return -48;
		}

		fromHasToken = false;
		// check fromtokens
   		for ( rapidjson::SizeType j = 0; j < fromdom.Size(); ++j) {
			rapidjson::Value &v = fromdom[j];
			if ( ! v.IsObject() ) {
				i("E32268 not object");
				return -55;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				i("E32208 no name");
				return -60;
			}
			const sstr &tname = itr->value.GetString();
			if ( tname != xferName ) {
				continue;
			}

			itr = v.FindMember("max");
			if ( itr == v.MemberEnd() ) {
				i("E32401 no max");
				return -61;
			}
			const sstr &max = itr->value.GetString();
			if ( atoll( max.c_str() ) == 1 ) {
				itr = v.FindMember("owner");
				if ( itr == v.MemberEnd() ) {
					i("E32471 nft token no owner");
					return -71;
				}
				const sstr &towner = itr->value.GetString();
				if ( from != towner ) {
					i("E32481 nft token wrong owner from=[%s] towner=[%s]", s(from), s(towner));
					return -71;
				}
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				i("E32204 no bal");
				return -65;
				//continue;
			}
			const sstr &bal = itr->value.GetString();
			if ( atof(bal.c_str()) < atof(xferAmount.c_str()) ) {
				i("E24722 error xferamount=[%s] iver bal=[%s]", xferAmount.c_str(), bal.c_str() );
				return -50;
			}

			fromHasToken = true;
			break;
		}

		if ( ! fromHasToken ) {
			i("E44023 fromacct has no such token [%s]", s(xferName) );
			return -55;
		}
	}

	return 0;
}

// Transfer tokens from one account to another
int BlockMgr::isXferTokenValid( OmicroTrxn &trxn)
{
	sstr trxnId; trxn.getTrxnID( trxnId );
	sstr from = trxn.sender_;
	sstr to = trxn.receiver_;
	d("a32007 transferToken trxnId=[%s] from=[%s] to=[%s]", s(trxnId), s(from), s(to) );

	OmstorePtr srcptr;
	char *fromrec = findSaveStore( from, srcptr );
	if ( ! fromrec ) {
		i("E30821 transferToken error from=[%s] not created yet.", s(from));
		return -10;
	}

	OmstorePtr dstptr;
	char *torec = findSaveStore( to, dstptr );
	if ( ! torec ) {
		i("E30321 transferToken error to=[%s] not created yet.", s(to));
		return -20;
	}

	OmAccount fromacct(fromrec);

	if ( fromacct.tokens_.size() < 1 ) {
		//acct.tokens_ = trxn.request_;
		i("E33401 from=[%s] tokens_ is empty", s(from) );
		return -30;
	}

	bool exist = OmToken::hasDupNames( fromacct.tokens_, trxn.request_);
	if ( ! exist ) {
		i("E22403 from=[%s] tokens_ does not have the tokens to be xfered", s(from) );
		i("E22303 fromacct.tokens_=[%s]", s(fromacct.tokens_) );
		i("E22403 trxn.request_=[%s]", s(trxn.request_) );
		return -40;
	}

	OmAccount toacct(torec);

	int rc = checkValidTokens(from, to, trxn.request_, fromacct.tokens_, toacct.tokens_ );
	if ( rc < 0 ) {
		i("E21403 from=[%s] to=[%s] checkValidTokens error rc=%d", s(from), s(to), rc );
		i("E21403 fromacct.tokens_=[%s]", s(fromacct.tokens_) );
		i("E21403 trxn.request_=[%s]", s(trxn.request_) );
		return -50;
	}

	return 0;
}

// may update requestJson
int BlockMgr::validateReqTokens( const sstr &from, sstr &requestJson )
{
	using namespace rapidjson;
	Document dom;
	dom.Parse( requestJson );
	if ( dom.HasParseError() ) {
		i("E38031 validateReqTokens invalid");
		return -10;
	}

	if ( ! dom.IsArray() ) {
		i("E38041 validateReqTokens dom is not array");
		return -20;
	}

	// each obj in dom
	bool hasChange = false;
	Value::ConstMemberIterator itr;
 	for ( rapidjson::SizeType j = 0; j < dom.Size(); ++j) {
			rapidjson::Value &v = dom[j];
			if ( ! v.IsObject() ) {
				i("E38061 error not object");
				return -25;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				i("E38051 no name found in req");
				return -27;
			}
			const sstr &tname = itr->value.GetString();
			if ( tname.size() < 1 ) {
				i("E38052 name empty in req");
				return -30;
			}
			if ( tname.size() > OM_TOKEN_MAX_LEN ) {
				i("E38052 name too long in req");
				return -35;
			}

			itr = v.FindMember("max");
			if ( itr == v.MemberEnd() ) {
				i("E38053 no max found in req");
				return -40;
			}
			const sstr &max = itr->value.GetString();
			if ( max.size() < 1 ) {
				i("E38054 max empty in req");
				return -50;
			}
			if ( max.size() > OM_NUM_MAXSZ ) {
				i("E38354 max too long in req");
				return -50;
			}

			if ( max.size() >0 && atoi( max.c_str() ) == 1 ) {
				itr = v.FindMember("owner");
					v.AddMember("owner", from, dom.GetAllocator() );
				if (  itr == v.MemberEnd() ) {
				} else {
					v["owner"].SetString( from, dom.GetAllocator() );
				}
				hasChange = true;
			}
	}

	if ( hasChange ) {
		StringBuffer sbuf;
		Writer<StringBuffer> writer(sbuf);
		dom.Accept(writer);
		requestJson = sbuf.GetString();
	}

	return 0;
}

void BlockMgr::setSrvPort( const sstr &srv, const sstr &port )
{
	srv_ = srv;
	port_ = port;
}
