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
#include <signal.h>
#include <cstdlib>
#include <pthread.h>
#include <malloc.h>
#include <boost/asio.hpp>

#include "omicrotrxn.h"
#include "omsession.h"
#include "server.hpp"
#include "dynamiccircuit.h"
#include "omutil.h"
#include "ommsghdr.h"
#include "omicroclient.h"
#include "omresponse.h"
#include "omlimits.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

EXTERN_LOGGING
using namespace boost::asio::ip;
using bcode = boost::system::error_code;

omsession::omsession(boost::asio::io_context& io_context, omserver &srv, tcp::socket socket)
        : io_context_(io_context), serv_(srv), socket_(std::move(socket))
{
    stop_ = false;
	clientIP_ = socket_.remote_endpoint().address().to_string();
	makeSessionID();
	hdr_[OMHDR_SZ] = '\0';
	i("I0001 accepted new client sid_=[%s] from %s:%d", s(sid_), s(clientIP_), socket_.remote_endpoint().port() );
}

void omsession::start()
{
    do_read();
	//printf("a33303 session start() done\n");
	//fflush( stdout );
}

omsession::~omsession()
{
	//printf("a33308 omsession dtor called\n");
	//fflush( stdout );
}

void omsession::do_read()
{
    auto self(shared_from_this());

    boost::asio::async_read( socket_, boost::asio::buffer(hdr_, OMHDR_SZ), 
          [this, self](bcode ec, std::size_t length)
          {
            // hanlder after reading data into data_
            if (!ec)  // no error
            {
				// do read data
				OmMsgHdr mhdr(hdr_, OMHDR_SZ, false);
				ulong dlen = mhdr.getLength();
				if ( dlen <= OM_MSG_MAXSZ ) {
    				char *data = (char*)malloc( dlen+1 );
    				data[dlen] = '\0';
    				d("a63003 a91838 srv doread dlen=%d length=%d hdr_[%s]", dlen, length, hdr_);
    
    				bcode ec2;
    				int len2 =  boost::asio::read( socket_, boost::asio::buffer(data,dlen), ec2 );
    				d("a45023 boost::asio::read len2=%d dlen=%d", len2, dlen );
    				data[len2] = '\0';
    
    				char tp = mhdr.getMsgType();
    				d("a10287 mhdr.getMsgType tp=[%c]", tp );
    				if ( tp == OM_TXN ) {
    					doTrxnL2( data, len2 );
    				} else if ( tp == OM_RQ ) {
    					doSimpleQuery( data, len2 );
    				} else if ( tp == OM_XNQ ) {
    					doQueryL2( data, len2 );
    				} else {
    					d("E30292 error invalid msgtype [%c]", tp );
    				}
    
    				free(data);
    				do_read();
				} else {
					i("E398462 dlen=%d too big > %d  close socket", dlen, OM_MSG_MAXSZ );
					socket_.close();
				}
            } else {
                d("a82838 srv do read read no data info=[%s]", ec.message().c_str());
			}
    });
}

void omsession::reply( const sstr &str, tcp::socket &socket )
{
	OmMsgHdr mhdr(hdr_, OMHDR_SZ, true);
	mhdr.setLength( str.size() );
	mhdr.setPlain();

    auto self(shared_from_this());

    boost::asio::async_write(socket, boost::asio::buffer(hdr_, OMHDR_SZ),
          [this, self, str, &socket](bcode ec, std::size_t len)
          {
            //d("a51550 111 in omsession::reply str=[%s] len=%d", s(str), str.size() );
            if (!ec)  // no error, OK
            {
				d("a43390 reply send async_write hdrmsg OK");
				try {
       				int len2 = boost::asio::write(socket, boost::asio::buffer(str.c_str(), str.size()));
    				d("a00292 server write str back str=[%s] strlen=%d done sentbytes=%d", str.c_str(), str.size(), len2 );
				} catch (std::exception& e) {
					i("E66331 error brokenpipe in server reply() exception [%s] clientIP_=[%s]", e.what(), s(clientIP_) );
				}
            } else {
                d("a33309 111 error async_write() srv reply hdr len=%d", len);
            }
    });
}

void omsession::doTrxnL2(const char *msg, int msglen)
{
	//d("a71002 doTrxn msg.len=%d msg=[%s]", msglen, msg );
	d("a71002 doTrxnL2 doTrxn msg.len=%d", msglen );
	sstr id_ = serv_.id_;

	OmicroTrxn t(msg);
	Byte xit = t.getXit();

	/**
	bool isInitTrxn1 = t.isInitTrxn();
	d("a71002 isInitTrxn1=%d", isInitTrxn1 );
	d("a71002 trxn.cipher=[%s]", s(t.cipher) );
	d("a71002 trxn.signature=[%s]", s(t.signature) );
	d("a028273 serv_.secKey_=[%s]", s(serv_.secKey_) );
	***/

	sstr trxnId; 
	bool isInitTrxn = t.isInitTrxn();
	t.getTrxnID( trxnId );

	d("a1000 from=[%s] trxntype=[%s] my srvport=%s peer=%s xit=%c txnid=%s isInitTrxn=%d", 
		s(t.sender_), s(t.trxntype_), s(serv_.srvport_), s(t.srvport_), xit, s(trxnId), isInitTrxn );

	sstr err;
	bool validTrxn = validateTrxn( trxnId, t, isInitTrxn, err );
	if ( ! validTrxn ) {
		if ( xit != XIT_i && xit != XIT_l && xit != XIT_m && xit != XIT_n ) {
			d("a334408 xit=%c not XIT_m XIT_n reply back", xit);
			sstr m;
			errResponse( trxnId, "INVALID_TRXN", id_ + " " + err, m );
			reply(m, socket_);
		}
		i("E40282 INVALID_TRXN ignore isInitTrxn=%d %s", isInitTrxn, s(err) );
		d("a1000 doTrxnL2 INVALID from=[%s] srvport=%s", s(t.sender_), s(serv_.srvport_)  );
		return;
	}

	bool rc;
	sstr peer = t.srvport_;
	sstr from = t.sender_;

	if ( isInitTrxn ) {
		d("a43713 init from=%s trxnId=[%s]", s(from), s(trxnId) );
		d("a333301  i am clientproxy, launching initTrxn ..." );
		if ( t.trxntype_ != OM_NEWACCT ) {
			sstr fence;
			serv_.blockMgr_.getFence( from, fence );
			t.fence_ = fence;
			d("a93910 isInitTrxn from=%s trxnid=%s  assign t.fence_=[%s]", s(from), s(trxnId), s(fence) );
		}

		rc = initTrxn( t );
		d("a333301  i am clientproxy, from=%s launched initTrxn rc=%d", s(from), rc );
		sstr m;
		if ( rc ) {
			okResponse( trxnId, "initTrxn", m );
		} else {
			errResponse( trxnId, "INVALID_TRXN", "initTrxn", m );
		}

		d("a333301 reply to endclient %s ...", s(m) );
		reply(m, socket_);
		d("a333301 from=%s reply to endclient %s done", s(from), s(m) );
	} else {
		d("a43714 from=%s exist trxnId=[%s]", s(from), s(trxnId) );
		sstr beacon = t.beacon_;
		if ( xit == XIT_i ) {
			d("a82208 from=%s %s recved XIT_i", s(from), s(sid_));
			// I got 'i', I must be a leader
			DynamicCircuit circ( serv_.nodeList_);
			strvec followers;
			bool iAmLeader = circ.isLeader( beacon, id_, true, followers );
			if ( iAmLeader ) {
				// state to A
				bool toAgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_i );
				if ( toAgood ) {
					d("a00233 from=%s XIT_i toAgood true trxnId=[%s]", s(from), s(trxnId) );

					// send XIT_j to all followers in this leader zone
					bool toBgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_j );
					d("a21200 from=%s iAmLeader XIT_j  toBgood=%d trxnId=%s", s(from), toBgood, s(trxnId) );

					t.setXit( XIT_j );
					strvec replyVec;
					d("a31112 from=%s %s multicast XIT_j followers for vote expect reply ..", s(from), s(sid_));
					//pvec( followers );

					//i("a949444 debug: followers:" );
					//pvectag( "tagfollower", followers );

					t.srvport_ = serv_.srvport_;
					sstr alldat; t.allstr(alldat);
					serv_.multicast( OM_TXN,  followers, alldat, false, replyVec ); // XIT_j
					d("a31112 from=%s %s multicast XIT_j followers for vote done replyVec=%d\n", s(from), s(sid_), replyVec.size() );
				} else {
					d("a3306 from=%s XIT_i to state A toAgood is false", s(from));
				}
			} else {
				// bad
				d("a3308 i [%s] am not leader, from=%s ignore XIT_i", s(sid_), s(from));
			}
			// else i am not leader, igore 'i' xit
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
			d("a31112 from=%s %s multicast XIT_j followers for vote expect reply ..", s(from), s(sid_));
			DynamicCircuit circ( serv_.nodeList_);
			strvec leader;
			bool isLeader = circ.getLeader(beacon, id_, leader );

			d("a10831 i am %s  my leader:", s(serv_.srvport_) );
			pvectag("tagfollowerleader:", leader );

			if ( ! isLeader ) {
				t.srvport_ = serv_.srvport_;
				t.setXit( XIT_k );
				strvec replyVec;
				sstr alldat; t.allstr(alldat);
				d("a34112 from=%s %s unicast XIT_k from follower to leader ..", s(from), s(sid_));
				serv_.multicast( OM_TXN, leader, alldat, false, replyVec );  // XIT_k
			} else {
				i("E33330 error got XIT_j but i am leader");
			}

		} else if ( xit == XIT_k ) {
			// if reaching quorum, fire next multicast
			d("a53103 from=%s %s got XIT_k peer:[%s]", s(from), s(id_), s(peer) );
			serv_.onRecvK( beacon, trxnId, clientIP_, sid_, t );
		} else if ( xit == XIT_l ) {
			d("a92822 from=%s %s received XIT_l peer:[%s] ...", s(from), s(sid_), s(peer) );
			serv_.onRecvL( beacon, trxnId, clientIP_, sid_, t );
			d("a92822 from=%s %s received XIT_l peer:[%s] done", s(from), s(sid_), s(peer) );
		} else if ( xit == XIT_m ) {
		    // received one XIT_m, there may be more XIT_m in next 3 seconds
			d("a54103 from=%s %s got XIT_m peer:[%s]", s(from), s(id_), s(peer) );
			serv_.onRecvM( beacon, trxnId, clientIP_, sid_, t );
			d("a54103 from=%s %s got XIT_m peer:[%s] done", s(from), s(id_), s(peer) );
		} else if ( xit == XIT_n ) {
			// follower gets a trxn commit message
			Byte curState; 
			serv_.trxnState_.getState( trxnId, curState );
			if ( curState != ST_F ) {
				i("a99 follower from=[%s] commit TRXN:%s peer:[%s]", s(from), s(trxnId), s(peer) );
				serv_.trxnState_.setState( trxnId, ST_F );  // to ST_F
				serv_.blockMgr_.receiveTrxn( t );
			} else {
				d("a99991 from=%s follower no-commit curState is already ST_F peer=[%s] for trxnid", s(from), s(peer));
			}
		} else {
			d("a200 doTrxnL2 from=[%s] xit is unknown [%c] peer:%s ", s(from), xit, s(serv_.srvport_)  );
		}
    }

	d("a1000 doTrxnL2 from=[%s] trxntype=[%s] my srvport=%s peer=%s xit=%c txnid=%s isInitTrxn=%d Done\n", 
		s(t.sender_), s(t.trxntype_), s(serv_.srvport_), s(t.srvport_), xit, s(trxnId), isInitTrxn );
}

void omsession::doSimpleQuery(const char *msg, int msglen)
{
	d("a71002 doSimpleQuery msg.len=%d msg=[%s]", msglen, msg );
	sstr id_ = serv_.id_;

    rapidjson::Document dom;
    dom.Parse( msg, msglen );
    if ( dom.HasParseError() ) {
        i("E43337 dom.HasParseError msg=[%s]\n", msg );
		sstr m;
		errResponse( "unknowTrxnId", "INVALID_QUERY", msg, m );
		reply( m, socket_ ); 
        return;
    }

    sstr qtype = dom["QT"].GetString();
    sstr trxnId = dom["TID"].GetString();
    sstr sender = dom["FRM"].GetString();
    sstr ts = dom["TS"].GetString();

	if ( qtype == "QP" ) {
		// request public key
		sstr json;
		okResponse( trxnId, serv_.pubKey_, json);
		reply( json, socket_ ); 
	} else if ( qtype == "QT" ) {
		// query trxn status
		sstr json;
		serv_.blockMgr_.queryTrxn( sender, trxnId, ts, json );
		reply( json, socket_ ); 
		d("a40088 received QT return res=[%s]", s(json));
	} else if ( qtype == "QQ" ) {
		d("a2939 QQ");
		// request last query result
		uint nodeLen = serv_.nodeList_.size();
		int votes = serv_.collectQueryKTrxn_.get(trxnId);
		d("a32209 nodeLen=%d votes=%d", nodeLen, votes );
		if ( uint(votes) < (2*nodeLen/3) ) {
			d("a040449 query not enough votes");
			sstr m;
			errResponse("NOTFOUND", trxnId, "NOT_ENOUGH_VOTES", m );
			reply( m, socket_ ); 
		} else {
			d("a040443 query got enough votes");
			sstr lastResult;
			getQueryResult( trxnId, sender, lastResult );
			reply( lastResult, socket_ ); 
		}

		/**
		sstr lastResult; // json
		getQueryResult( trxnId, sender, lastResult );
		d("a43207 lastResult=[%s] reply...", s(lastResult) );
		reply( lastResult, socket_ ); 
		d("a43207 lastResult=[%s] sent done", s(lastResult) );
		**/
	} else {
		sstr m;
		errResponse( trxnId, "INVALID_REQUEST", msg, m );
		reply( m, socket_ ); 
	}

	d("a535023 doSimpleQuery done clientIP_=[%s]", s(clientIP_));
}

bool omsession::initTrxn( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// serv_.nodeList_ is std::vector<string>
	sstr beacon = txn.beacon_;
	sstr trxnid; txn.getTrxnID( trxnid );
	d("a80123 initTrxn() threadid=%ld beacon=[%s] trxnid=[%s]", pthread_self(), s(beacon), s(trxnid) );

	// for each zone leader
	//   send leader msg: trxn, with tranit XIT_i
	// self node maybe one of the zone leaders
	DynamicCircuit circ(serv_.nodeList_);
	strvec hostVec;
	circ.getZoneLeaders( beacon, hostVec );

	for ( auto &id: hostVec ) {
		d("a20112 initTrxn send XIT_i to leader [%s]", s(id) );
	}

	txn.setNotInitTrxn();
	txn.setXit( XIT_i );

	strvec replyVec;
	d("a31181 multicast to ZoneLeaders expectReply=false ...");
	//pvec( hostVec );
	txn.srvport_ = serv_.srvport_;
	sstr alldat; txn.allstr(alldat);
	int connected = serv_.multicast( OM_TXN, hostVec, alldat, false, replyVec );
	d("a31183 multicast to ZoneLeaders done connected=%d", connected);

	uint twofp1 = twofplus1(hostVec.size());
	if ( uint(connected) >= twofp1 ) {
		d("a4221 connected=%u >= onefp1=%u true", uint(connected), twofp1 );
		return true;
	} else {
		d("a4222 connected=%u < onefp1=%u false", uint(connected), twofp1 );
		return true;
	}
}

void omsession::makeSessionID()
{
    struct timeval now;
    gettimeofday( &now, NULL );
	char buf[16];
	sprintf(buf, "%d%ld", int(now.tv_sec%10),  now.tv_usec);
	sid_ = buf;
}

bool omsession::validateTrxn( const sstr &trxnId,  OmicroTrxn &txn, bool isInitTrxn, sstr &err )
{
	d("a17621 omsession::validateTrxn serv_.address=[%s] serv_.port=[%s]", s(serv_.address_), s(serv_.port_) );
	//d("22206 my serv_.pubKey_=[%s]", s(serv_.pubKey_) );
	//d("22206 my serv_.secKey_=[%s]", s(serv_.secKey_) );

	// check state if ST_F then all done, ignore trxn
	Byte curState;
	serv_.trxnState_.getState( trxnId, curState );
	if ( curState >= ST_F ) {
		d("E30280 trxn is done in ST_F state.trxnid=[%s]", s(txn.sender_), s(trxnId)  );
		err = sstr("Late trxn request ") + serv_.address_ + ":" + serv_.port_;
		return false;
	}

	bool validTrxn = txn.validateTrxn( serv_.secKey_ );
	if ( ! validTrxn ) {
		i("E30290 trxn is invalid. I am node: %s  trxnid=[%s]", s(serv_.srvport_), s(trxnId)  );
		i("E30290 trxn invalid. sender node: %s", s(txn.srvport_) );
		err = sstr("Transaction object invalid ") + serv_.address_ + ":" + serv_.port_;
		return false;
	}

	if ( txn.trxntype_ == OM_PAYMENT ) {
		double bal;
		sstr pubkey;
		int rc = serv_.blockMgr_.getBalanceAndPubkey( txn.sender_, bal, pubkey );
		if ( rc < 0 ) {
            i("E32018 error from=[%s] invalid rc=%d", s(txn.sender_), rc );
			err = sstr("Unable to get balance and public key of sender [") + txn.sender_ + "] in PAYMENT";
            return false;
		}

		if ( pubkey != txn.userPubkey_ ) {
            i("E32016 pubkey mismatch from=[%s] pubkey ", s(txn.sender_) );
            i("E32016 txn.pubkey=[%s]", s(txn.userPubkey_) );
            i("E32016 acct.pubkey=[%s]", s(pubkey) );
			err = "Public key mismatch";
            return false;
		}

    	double amt = txn.getAmountDouble();
        d("a44502 from=[%s] balance=[%.6g]", s(txn.sender_), bal );
        if ( (bal - amt) < 0.0001 ) {
            d("a30138 from=[%s] balance=%.6g not enough to pay %.6g", s(txn.sender_), bal, amt );
			err = "Not enough funds";
            return false;
        }

		if ( ! isInitTrxn ) {
			sstr fromFence;
			serv_.blockMgr_.getFence( txn.sender_, fromFence);
			if ( txn.fence_ != fromFence ) {
				std::vector<sstr> vec;
				char tstat;
				sstr errmsg;
				serv_.blockMgr_.readTrxns( txn.sender_, txn.timestamp_, trxnId, vec, tstat, errmsg );
				if ( vec.size() > 0 ) {
					d("a900123 fencing diff, and readTrxns is found, size=%d mean trxnId exist alreay, return false", vec.size() );
					err = "Fencing error";
           			i("E32012 error from=[%s] txn.fence_=[%s] NE fromFence=[%s] trxnid=%s", s(txn.sender_), s(txn.fence_), s(fromFence), s(trxnId) );
           			i("E32012 error from=[%s] fence txn.srvport_=[%s]", s(txn.sender_), s(txn.srvport_) );
					return false;
				} else {
					d("a900123 fencing diff, but blockMgr_.readTrxns not found brc=%d, go ahead errmsg=%s", s(errmsg));
				}
			}
		}
	} else if ( txn.trxntype_ == OM_XFERTOKEN ) {
		int rc = serv_.blockMgr_.isXferTokenValid( txn );
		if ( rc < 0 ) {
           	i("E35012 from=[%s] xfer token invalid ", s(txn.sender_), s(txn.receiver_)  );
			err = "Transfer error";
			return false;
		}

		if ( ! isInitTrxn ) {
			sstr fromFence;
			serv_.blockMgr_.getFence( txn.sender_, fromFence);
			if ( txn.fence_ != fromFence ) {
				std::vector<sstr> vec;
				char tstat;
				sstr errmsg;
				serv_.blockMgr_.readTrxns( txn.sender_, txn.timestamp_, trxnId, vec, tstat, errmsg );
				if ( vec.size() > 0 ) {
					err = "Fencing error";
           			i("E32014 error from=[%s] txn.fence_=[%s] NE fromFence=[%s] trxnid=%s", s(txn.sender_), s(txn.fence_), s(fromFence), s(trxnId) );
           			i("E32014 error from=[%s] fence txn.srvport_=[%s]", s(txn.sender_), s(txn.srvport_) );
					return false;
				} else {
					d("a900124 fencing diff, but blockMgr_.readTrxns not found brc=%d, go ahead errmsg=%s", s(errmsg));
				}
			}
		}
	}

	return true;
}

bool omsession::validateQuery( OmicroTrxn &txn, const sstr &trxnId, bool isInitTrxn, sstr &err )
{
	d("a82220 enter omsession::validateQuery");
	d("22208 my serv_.pubKey_=[%s]", s(serv_.pubKey_) );
	d("22208 my serv_.secKey_=[%s]", s(serv_.secKey_) );

	bool validTrxn = txn.validateTrxn( serv_.secKey_ );
	if ( ! validTrxn ) {
		i("E20290 validateQuery query is not valid");
		err = "Invalid tranaction message";
		return false;
	}

	// perform query get result
	sstr res;
	int rc = serv_.blockMgr_.runQuery(txn, res);
	if  ( rc < 0 ) {
		i("E40021 runQuery error rc=%d  request=[%s]", rc, s(txn.request_) );
		err = "Query error";
		return false;
	}

	if ( isInitTrxn ) {
		txn.response_ = res;
		d("a38800 after runQuery OK");
		return true;
	}

	if ( txn.response_ != res ) {
		d("a31800 after runQuery  txn.response_ != res ");
		d("a1004 txn.response_=[%s]", s(txn.response_) );
		d("a1004 res =[%s]", s(res) );
		err = "Query result mismatch";
		return false;
	} else {
		d("a31400 after runQuery  txn.response_ == res OK ");
		serv_.qResult_.add(trxnId, res);
		d("a31400 after runQuery sid_=[%s]  trxnId=[%s] res=[%s]", s(sid_),  s(trxnId), s(res) );
		return true;
	}

}

void omsession::doQueryL2(const char *msg, int msglen)
{
	d("a71003 doQueryL2 msg.len=%d", msglen );
	sstr id_ = serv_.id_;

	OmicroTrxn t(msg);
	/**
	bool isInitTrxn1 = t.isInitTrxn();
	d("a71002 isInitTrxn1=%d", isInitTrxn1 );
	d("a71002 trxn.cipher=[%s]", s(t.cipher) );
	d("a71002 trxn.signature=[%s]", s(t.signature) );
	d("a028273 serv_.secKey_=[%s]", s(serv_.secKey_) );
	***/

	sstr trxnId; 
	bool isInitTrxn = t.isInitTrxn();
	t.trxntype_ = OM_QUERY;

	t.getTrxnID( trxnId );

	sstr err;
	bool validTrxn = validateQuery( t, trxnId, isInitTrxn, err );
	// run query in validateQuery
	if ( ! validTrxn ) {
		sstr json;
		errResponse( trxnId, "INVALID_QUERY", id_ + "|" + err, json );
		reply(json, socket_);
		i("E40202 INVALID_TRXN query ignore" );
		return;
	}

	bool rc;
	sstr peer = t.srvport_;
	sstr m;

	if ( isInitTrxn ) {
		//t.setID();
		d("a41713 initquery trxnId=[%s]", s(trxnId) );

		rc = initQuery( t );

		OmResponse resp;
		resp.TID_ = trxnId;
		if ( rc ) {
			resp.STT_ = OM_RESP_OK;
			resp.RSN_ = "initQuery";
		} else {
			resp.STT_ = OM_RESP_ERR;
			resp.RSN_ = "INVALID_QUERY";
		}
		resp.json( m );
		reply(m, socket_);

	} else {
		Byte xit = t.getXit();
		d("a43712 exist trxnId=[%s] xit=[%c]", s(trxnId), xit );
		sstr beacon = t.beacon_;
		if ( xit == XIT_i ) {

			d("a84213 %s recved XIT_i", s(sid_));
			// I got 'i', I must be a leader
			DynamicCircuit circ( serv_.nodeList_);
			strvec followers;
			bool iAmLeader = circ.isLeader( beacon, id_, true, followers );
			if ( iAmLeader ) {
				// state to A
				bool toAgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_i );
				if ( toAgood ) {
					d("a01233 XIT_i toAgood ST_A true trxnid=[%s]", s(trxnId));

					// send XIT_j to all followers in this leader zone
					bool toBgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_j );
					d("a22200 iAmLeader XIT_j  toBgood=%d trxnid=[%s]", toBgood, s(trxnId) );

					t.setXit( XIT_j );
					strvec replyVec;
					d("a32112 %s multicast XIT_j followers for vote expect reply ..", s(sid_));

					//i("a949449 debug: followers:" );
					//pvectag( "tagfollower", followers );

					t.srvport_ = serv_.srvport_;
					sstr alldata; t.allstr(alldata);
					serv_.multicast( OM_XNQ, followers, alldata, false, replyVec ); // sending XIT_j
					d("a32112 %s multicast XIT_j followers for vote done replyVec=%d\n", s(sid_), replyVec.size() );
				} else {
					d("a3306 XIT_i to state A toAgood is false");
				}
			} else {
				// bad
				d("a3308 i [%s] am not leader, ignore XIT_i", s(sid_));
			}
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
			DynamicCircuit circ( serv_.nodeList_);
			strvec leader;
			strvec replyVec;
			bool iAmLeader = circ.getLeader( beacon, id_, leader );
			if ( ! iAmLeader ) {
				t.setXit( XIT_k );
				t.srvport_ = serv_.srvport_;
				sstr alldata; t.allstr(alldata);
				serv_.multicast( OM_XNQ, leader, alldata, false, replyVec ); // sending XIT_k
			}
		} else if ( xit == XIT_k ) {
			serv_.onRecvQueryK( beacon, trxnId, clientIP_, sid_, t );
		}
    }

	d("a555024 doQueryL2 done clientIP_=[%s]", s(clientIP_));
}  // doQueryL2

bool omsession::initQuery( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// serv_.nodeList_ is std::vector<string>
	sstr beacon = txn.beacon_;
	sstr trxnid; txn.getTrxnID( trxnid );
	d("a80120 tQueryinitTrxn() threadid=%ld beacon=[%s] trxnid=[%s]", pthread_self(), s(beacon), s(trxnid) );

	// for each zone leader
	//   send leader msg: trxn, with tranit XIT_i
	// self node maybe one of the zone leaders
	DynamicCircuit circ(serv_.nodeList_);
	strvec hostVec;
	circ.getZoneLeaders( beacon, hostVec );

	// todo
	for ( auto &id: hostVec ) {
		d("a20122 initTrxn send XIT_i to leader [%s]", s(id) );
	}

	txn.setNotInitTrxn();
	txn.setXit( XIT_i );
	txn.srvport_ = serv_.srvport_;

	strvec replyVec;
	d("a31182 multicast to ZoneLeaders expectReply=false ...");
	//pvec( hostVec );
	txn.srvport_ = serv_.srvport_;

	int connected;
	uint twofp1;
	bool expectReply = false;

	sstr alldat; txn.allstr(alldat);
	connected = serv_.multicast( OM_XNQ, hostVec, alldat, expectReply, replyVec );
	if ( expectReply ) {
		twofp1 = twofplus1(replyVec.size());
	} else {
		twofp1 = twofplus1(hostVec.size());
	}

	d("a31174 multicast to ZoneLeaders done connected=%d", connected);

	if ( uint(connected) >= twofp1 ) {
		d("a4321 connected=%u >= onefp1=%u true", uint(connected), twofp1 );
		return true;
	} else {
		d("a4322 connected=%u < onefp1=%u false", uint(connected), twofp1 );
		return false;
	}
}

// res is json retured
void omsession::getQueryResult( const sstr &trxnId, const sstr &sender, sstr &res )
{
	OmResponse resp;
	resp.TID_ = trxnId;
	resp.UID_ = sender;
	resp.NID_ = serv_.id_;

	res = "";
	serv_.qResult_.get( trxnId, res );

	if ( res.size() < 1 ) {
		resp.STT_ = OM_RESP_ERR;
		resp.RSN_ = "NOTFOUND";
		d("a23309  sid_=[%s]  trxnId=[%s] not found", s(sid_), s(trxnId) );
	} else {
		resp.STT_ = OM_RESP_OK;
		resp.DAT_ = res;
	}

	resp.json( res );
}

