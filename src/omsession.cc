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
	d("a00001 newsession sid_=[%s]", s(sid_) );
}

void omsession::start()
{
    do_read();
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
                d("a82838 srv do read read no data error=[%s]", ec.message().c_str());
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
					i("a66331 error brokenpipe in server reply() exception [%s] clientIP_=[%s]", e.what(), s(clientIP_) );
				}
            } else {
                d("a33309 111 error async_write() srv reply hdr len=%d", len);
            }
    });
}

void omsession::doTrxnL2(const char *msg, int msglen)
{
	//d("a71002 doTrxn msg.len=%d msg=[%s]", msglen, msg );
	d("a71002 doTrxn msg.len=%d", msglen );
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
	t.getTrxnID( trxnId );

	sstr err;
	bool validTrxn = validateTrxn( t, isInitTrxn, err );
	if ( ! validTrxn ) {
		// sstr m = sstr("INVALID_TRXN|") + id_ + "|" + err;
		sstr m;
		errResponse( trxnId, "INVALID_TRXN", id_ + " " + err, m );
		reply(m, socket_);
		i("E40282 INVALID_TRXN ignore isInitTrxn=%d", isInitTrxn );
		return;
	}

	bool rc;
	sstr pfrom = t.srvport_;

	if ( isInitTrxn ) {
		t.setID();
		// t.getTrxnID( trxnId );
		d("a43713 init trxnId=[%s]", s(trxnId) );
		d("a333301  i am clientproxy, launching initTrxn ..." );
		// add fence_ of sender
		sstr fence;
		serv_.blockMgr_.getFence( t.sender_, fence );
		t.fence_ = fence;
		rc = initTrxn( t );
		d("a333301  i am clientproxy, launched initTrxn rc=%d", rc );
		sstr m;
		if ( rc ) {
			//m = sstr("GOOD_TRXN|initTrxn|") +trxnId + "|t33093|" + sid_;
			okResponse( trxnId, "initTrxn", m );
		} else {
			//m = sstr("INVALID_TRXN|initTrxn|") +trxnId + "|t531019|" + sid_;
			errResponse( trxnId, "INVALID_TRXN", "initTrxn", m );
		}

		d("a333301 reply to endclient %s", s(m) );
		reply(m, socket_);
		d("a333301 reply to endclient %s done", s(m) );
	} else {
		//t.getTrxnID( trxnId );
		d("a43714 exist trxnId=[%s]", s(trxnId) );
		Byte xit = t.getXit();
		sstr beacon = t.beacon_;
		if ( xit == XIT_i ) {

			d("a82208 %s recved XIT_i", s(sid_));
			// I got 'i', I must be a leader
			DynamicCircuit circ( serv_.nodeList_);
			strvec followers;
			bool iAmLeader = circ.isLeader( beacon, id_, true, followers );
			if ( iAmLeader ) {
				// state to A
				bool toAgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_i );
				if ( toAgood ) {
					d("a00233 XIT_i toAgood true");

					// send XIT_j to all followers in this leader zone
					bool toBgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_j );
					d("a21200 iAmLeader XIT_j  toBgood=%d", toBgood );

					t.setXit( XIT_j );
					strvec replyVec;
					d("a31112 %s multicast XIT_j followers for vote expect reply ..", s(sid_));
					pvec( followers );
					t.srvport_ = serv_.srvport_;
					sstr alld; t.allstr(alld);
					serv_.multicast( OM_TXN,  followers, alld, true, replyVec );
					d("a31112 %s multicast XIT_j followers for vote done replyVec=%d\n", s(sid_), replyVec.size() );

					// got replies from followers, state to C
					bool toCgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_k );
					if ( toCgood ) {
						// d("a55550 recv XIT_k toCgood true");
						d("a55550 received all replies of XITT_j toCgood true");
						if ( serv_.level_ == 2 ) {
							int votes = replyVec.size(); // how many replied
							t.setVoteInt( votes );
							// send el-'l' xit to other leaders
							t.setXit( XIT_l );
							strvec otherLeaders;
							circ.getOtherLeaders( beacon, id_, otherLeaders );
							d("a31102 %s round-1 multicast XIT_l otherLeaders noreplyexpected ..", s(sid_));
							pvec(otherLeaders);
							// txn.setSrvPort( serv_.srvport_.c_str() );
							sstr dat; t.allstr(dat);
							serv_.multicast( OM_TXN, otherLeaders, dat, false, replyVec );
							d("a31102 %s round-1 multicast XIT_l otherLeaders done replyVec=%d\n", s(sid_), replyVec.size() );
							// XIT_m should be in reply
							// if there are enough replies, multicase XIT_n to followers
							d("a43330 XIT_m should be in reply multicase XIT_n to followers ...");
						} else {
							// level_ == 3  todo
							d("a63311 error level_ == 2 false");
						}

					} else {
						d("a3305 XIT_j toCgood is false");
					}
				} else {
					d("a3306 XIT_i to state A toAgood is false");
				}
			} else {
				// bad
				d("a3308 i [%s] am not leader, ignore XIT_i", s(sid_));
			}
			// else i am not leader, igore 'i' xit
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
			d("a5501 received XIT_j from [%s] reply back good", s(pfrom));
			// sstr m = sstr("GOOD_TRXN|XIT_j|")+id_ + "|" + sid_;;
			sstr m;
			okResponse(trxnId, "XIT_j", m );
			d("a555550 ok m=[%s]", m.c_str() );
			reply(m, socket_);
		} else if ( xit == XIT_l ) {
			d("a92822 %s received XIT_l from [%s] ...", s(sid_), s(pfrom) );
			serv_.onRecvL( beacon, trxnId, clientIP_, sid_, t );
		} else if ( xit == XIT_m ) {
		    // received one XIT_m, there may be more XIT_m in next 3 seconds
			d("a54103 %s got XIT_m from [%s]", s(id_), pfrom );
			serv_.onRecvM( beacon, trxnId, clientIP_, sid_, t );
		} else if ( xit == XIT_n ) {
			// follower gets a trxn commit message
			d("a9999 follower commit a TRXN %s from [%s]", s(trxnId), s(pfrom));
			serv_.blockMgr_.receiveTrxn( t );
		} else if ( xit == XIT_z ) {
			/***
			// query trxn status
			sstr res;
			serv_.blockMgr_.queryTrxn( t.sender, trxnId, t.timestamp, res );
			reply( res, socket_ ); 
			d("a40088 received XIT_z return res");
			***/
		}
    }

	d("a555023 doTrxnL2 done clientIP_=[%s]", s(clientIP_));
}

void omsession::doSimpleQuery(const char *msg, int msglen)
{
	// d("a71002 doSimpleQuery msg.len=%d msg=[%s]", msglen, msg );
	sstr id_ = serv_.id_;

    rapidjson::Document dom;
    dom.Parse( msg );
    if ( dom.HasParseError() ) {
        printf("E43337 dom.HasParseError msg=[%s]\n", msg );
		sstr m;
		errResponse( "unknowTrxnId", "INVALID_QUERY", msg, m );
		reply( m, socket_ ); 
        return;
    }

    sstr qtype = dom["QT"].GetString();
    sstr trxnId = dom["TID"].GetString();
    sstr sender = dom["FRM"].GetString();
    sstr ts = dom["TS"].GetString();

	if ( qtype == "QT" ) {
		// query trxn status
		sstr json;
		serv_.blockMgr_.queryTrxn( sender, trxnId, ts, json );
		reply( json, socket_ ); 
		d("a40088 received QT return res");
	} else if ( qtype == "QP" ) {
		// request public key
		sstr json;
		okResponse( trxnId, serv_.pubKey_, json);
		reply( json, socket_ ); 
	} else if ( qtype == "QQ" ) {
		d("a2939 QQ");
		// request last query result
		// todo check server queryVote_ for trxnId
		uint nodeLen = serv_.nodeList_.size();
		int votes;
		serv_.getQueryVote( trxnId, votes );
		d("a32209 nodeLen=%d votes=%d", nodeLen, votes );
		/***
		if ( uint(votes) < (2*nodeLen/3) ) {
			sstr m = trxnId + "|NOTFOUND";
			reply( m, socket_ ); 
		} else {
			sstr lastResult;
			getResult( trxnId, lastResult );
			reply( lastResult, socket_ ); 
		}
		***/
		sstr lastResult; // json
		getResult( trxnId, sender, lastResult );
		d("a43207 lastResult=[%s] reply...", s(lastResult) );
		reply( lastResult, socket_ ); 
		d("a43207 lastResult=[%s] sent done", s(lastResult) );
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
	pvec( hostVec );
	txn.srvport_ = serv_.srvport_;
	sstr dat; txn.allstr(dat);
	int connected = serv_.multicast( OM_TXN, hostVec, dat, false, replyVec );
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

bool omsession::validateTrxn( OmicroTrxn &txn, bool isInitTrxn, sstr &err )
{
	bool validTrxn = txn.validateTrxn( serv_.secKey_ );
	if ( ! validTrxn ) {
		i("E30290 trxn is not valid");
		err = "Transaction object invalid";
		return false;
	}

	if ( txn.trxntype_ == OM_PAYMENT ) {
		double bal;
		sstr pubkey;
		int rc = serv_.blockMgr_.getBalanceAndPubkey( txn.sender_, bal, pubkey );
		if ( rc < 0 ) {
            i("E32018 from=[%s] invalid rc=%d", s(txn.sender_), rc );
			err = "Unable to get balance and public key of sender";
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
        d("a44502 from=[%s] balance=[%.6f]", s(txn.sender_), bal );
        if ( (bal - amt) < 0.0001 ) {
            d("a30138 from=[%s] balance=%.6f not enough to pay %.6f", s(txn.sender_), bal, amt );
			err = "Not enough funds";
            return false;
        }

		if ( ! isInitTrxn ) {
			sstr fromFence;
			serv_.blockMgr_.getFence( txn.sender_, fromFence);
			if ( txn.fence_ != fromFence ) {
            	i("E32012 from=[%s] txn.fence_=[%s] != fromFence=[%s]", s(txn.sender_), s(txn.fence_), s(fromFence) );
				err = "Fencing error";
				return false;
			}

		}
	}

	return true;
}

bool omsession::validateQuery( OmicroTrxn &txn, const sstr &trxnId, bool isInitTrxn, sstr &err )
{
	bool validTrxn = txn.validateTrxn( serv_.secKey_ );
	if ( ! validTrxn ) {
		i("E20290 query is not valid");
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
		serv_.result_[trxnId] = res;
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
	if ( ! validTrxn ) {
		sstr json;
		errResponse( trxnId, "INVALID_QUERY", id_ + "|" + err, json );
		reply(json, socket_);
		i("E40202 INVALID_TRXN query ignore" );
		return;
	}

	bool rc;
	sstr pfrom = t.srvport_;
	sstr m;

	if ( isInitTrxn ) {
		t.setID();
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
					d("a01233 XIT_i toAgood true");

					// send XIT_j to all followers in this leader zone
					bool toBgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_j );
					d("a22200 iAmLeader XIT_j  toBgood=%d", toBgood );

					t.setXit( XIT_j );
					strvec replyVec;
					d("a32112 %s multicast XIT_j followers for vote expect reply ..", s(sid_));
					pvec( followers );
					t.srvport_ = serv_.srvport_;
					sstr alld; t.allstr(alld);
					serv_.multicast( OM_XNQ, followers, alld, true, replyVec );
					d("a32112 %s multicast XIT_j followers for vote done replyVec=%d\n", s(sid_), replyVec.size() );

					int votes = replyVec.size(); // how many replied
					t.setVoteInt( votes );
					serv_.addQueryVote( trxnId, votes );
				} else {
					d("a3306 XIT_i to state A toAgood is false");
				}
			} else {
				// bad
				d("a3308 i [%s] am not leader, ignore XIT_i", s(sid_));
			}
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
			d("a5502 received XIT_j from [%s] reply back good", s(pfrom));
			sstr json;
			okResponse( trxnId, "XIT_j", json);
			reply(json, socket_);
			d("a55150 m=[%s]", json.c_str() );

			/***
			bool toBgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_j );
			d("a55150 received T_j  toBgood=%d, reply back xtrn T_k ...", toBgood );
			t.setXit( XIT_k );
			t.srvport_ = serv_.srvport_;
			sstr alld; t.allstr(alld);
			reply(alld, socket_);
			***/
		} else if ( xit == XIT_k ) {
			//qwer
			// need rendezvous for all followers to ge to state 'C'
			//serv_.onRecvK( beacon, trxnId, clientIP_, sid_, t );
			// send to clientproxy client message GOOD
		}
    }

	d("a555023 doQueryL2 done clientIP_=[%s]", s(clientIP_));
}

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

	for ( auto &id: hostVec ) {
		d("a20122 initTrxn send XIT_i to leader [%s]", s(id) );
	}

	txn.setNotInitTrxn();
	txn.setXit( XIT_i );

	strvec replyVec;
	d("a31182 multicast to ZoneLeaders expectReply=false ...");
	pvec( hostVec );
	txn.srvport_ = serv_.srvport_;
	sstr dat; txn.allstr(dat);

	int connected;
	uint twofp1;
	bool expectReply = false;

	connected = serv_.multicast( OM_XNQ, hostVec, dat, expectReply, replyVec );
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
void omsession::getResult( const sstr &trxnId, const sstr &sender, sstr &res )
{
	auto itr = serv_.result_.find( trxnId );
	OmResponse resp;
	resp.TID_ = trxnId;
	resp.UID_ = sender;
	resp.NID_ = serv_.id_;

	if ( itr == serv_.result_.end() ) {
		resp.STT_ = OM_RESP_ERR;
		resp.RSN_ = "NOTFOUND";
		d("a23309  sid_=[%s]  trxnId=[%s] not found", s(sid_), s(trxnId) );
	} else {
		resp.STT_ = OM_RESP_OK;
		resp.DAT_ = itr->second;
	}

	resp.json( res );
}

