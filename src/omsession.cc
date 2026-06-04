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
#include "ZlibCompress.h"

/**************************************************************************
**  Transaction: transfer, creation of tokens
**  Query:       lookup account and token information
***************************************************************************/

/**************************************************************************
**
**  Session object receivs commands from clients or peers and process
**  the commands. A server node may take multiple sessions simultaneouly.
**  Each session contains a connection. A session may take several commands
**  sequentially but in most cases a session takes only one message.
**
**************************************************************************/
EXTERN_LOGGING
using namespace boost::asio::ip;
using bcode = boost::system::error_code;

OmSession::OmSession(boost::asio::io_context& io_context, OmServer &srv, tcp::socket socket)
        : io_context_(io_context), serv_(srv), socket_(std::move(socket))
{
    stop_ = false;
    bcode  ec;
    tcp::endpoint ep1 = socket_.remote_endpoint(ec);
    // tcp::endpoint ep2 = socket_.local_endpoint(ec2);

    int sockfd = socket_.native_handle();
    d("a555039 in OmSession ctor socket_.native_fd sockfd=%d", sockfd);


    myIP_ = srv.address_;  // string
    myPort_ = static_cast<unsigned int>( std::stoul(srv.port_));

    if ( ! ec ) {
	    clientIP_ = ep1.address().to_string();
        clientPort_ = ep1.port();
	    makeSessionID();
	    hdr_[OMHDR_SZ] = '\0';
	    d("I0001 accepted new client sid_=[%s] from %s:%d", s(sid_), s(clientIP_), socket_.remote_endpoint().port() );
    } else {
        clientIP_ = "UNKNOWN";
        clientPort_ = 0;
        sid_ = "";
    }
}

void OmSession::start()
{
    d("a2229393 OmSession::start() do_read ...");
    do_read();
}

OmSession::~OmSession()
{
    d("a211209 destructor of OmSession ");
    int sockfd = socket_.native_handle();
    d("a555039 in OmSession dtor socket_.native_fd sockfd=%d close", sockfd);

    socket_.close();
}

// start to accept commands
void OmSession::do_read()
{
    auto self(shared_from_this());

    boost::asio::async_read( socket_, boost::asio::buffer(hdr_, OMHDR_SZ), 
          [this, self ](bcode ec, std::size_t length)
          {
            // hanlder after reading data into data_
            if (!ec)  // no error
            {
    			//d("a63003 async_read hdr=[%s] length=%lu", hdr, length);
				OmMsgHdr mhdr(hdr_, OMHDR_SZ, false);
				ulong dlen = mhdr.getLength();
				if ( dlen <= OM_MSG_MAXSZ ) {
    				char *data = (char*)malloc( dlen+1 );
                    memset( data, 0, dlen+1 );
    				data[dlen] = '\0';
    
    				bcode ec2;
    				int len2 =  boost::asio::read( socket_, boost::asio::buffer(data,dlen), ec2 );
                    if ( ! ec2 ) {  // no error
        				d("a45023 boost::asio::read got new command len2=%d dlen=%d ec2=%d", len2, dlen, ec2.value() );
        				data[len2] = '\0';
                        d("a45023 read/receved message is [%.257s]", data );
    
    					sstr msg(data, len2);
    					free(data);
                       	sstr uncomp;
    
            			char c = mhdr.getCompression();
                    	if ( c == OM_COMPRESSED ) {
                        	d("a33185 srv got OM_COMPRESSED unzip");
                        	ZlibCompress::uncompress(msg, uncomp);
                        	data = (char*)uncomp.c_str();
    						len2 = uncomp.size();
        					d("a45025 after uncompress len2=%d", len2 );
                    	} else {
                        	d("a33186 srv not OM_COMPRESSED");
    						data = (char*)msg.c_str();
    						// len2 no change
    					}
        
        				char tp = mhdr.getMsgType();
        				d("a10287 mhdr.getMsgType tp=[%c]", tp );
        				if ( tp == OM_TXN ) {
                            d("a60034 start doTrxnL2() ...");
        					doTrxnL2( data, len2 );
                            d("a60034 doTrxnL2() done");
        				} else if ( tp == OM_RQ ) {
                            d("a60035 start doSimpleQuery() ...");
        					doSimpleQuery( data, len2 );
                            d("a60035 doSimpleQuery() done");
        				} else if ( tp == OM_XNQ ) {
                            d("a60036 start doQueryL2() ...");
        					doQueryL2( data, len2 );
                            d("a60036 doQueryL2() done");
        				} else {
        					d("E30292 error invalid msgtype [%c]", tp );
        				}
    
    				    d("a63003 async_read and processing done. do do_read() again for next command\n\n" );
    				    do_read();
                    } else {
					    d("E394462 error ec2=%d close socket", ec2.value() );
					    socket_.close();
                    }
				} else {
					d("E398462 error dlen=%d too big > %d  close socket", dlen, OM_MSG_MAXSZ );
					socket_.close();
				}
            } else {
                d("a82838 srv do read read no data info=[%s]", ec.message().c_str());
				if ( ec == boost::asio::error::eof ) {
                    int sockfd = socket_.native_handle();
					d("a33330 eof close socket sockfd=%d", sockfd);
					socket_.close();
				}
			}
    });
}

// Send reply back to peer. Mostly it sends short messages, but
// sometimes it may send a transaction message to a peer.
void OmSession::reply( const sstr &str, tcp::socket &socket )
{
    #if 0
	char hdr[OMHDR_SZ+1];
	hdr[OMHDR_SZ] = '\0'; 

	bool doCompress = false;
	OmMsgHdr mhdr(hdr, OMHDR_SZ, true);

    // int doCompressThreshold = 1000;
    ulong doCompressThreshold = 300000;

	sstr zip;
	if ( str.size() > doCompressThreshold ) {
		doCompress = true;
		ZlibCompress::compress( str, zip);
		mhdr.setLength( zip.size() );
		mhdr.setCompressed();
	} else {
		zip = str;
		mhdr.setLength( str.size() );
		mhdr.setPlain();
	}

    auto self(shared_from_this());

    // async_write is async, the current function returns immediately, then the actual write may happen later
    // (imagine the actual write can take 3 days later), which will fills value to ec and len.
    boost::asio::async_write(socket, boost::asio::buffer(hdr, OMHDR_SZ),
          [this, self, &socket, doCompress, zip](bcode ec, std::size_t len)
          {
            //d("a51550 111 in OmSession::reply str=[%s] len=%d", s(str), str.size() );
            int sockfd = socket_.native_handle();
            if (!ec)  // no error, OK
            {
				d("a43390 reply send async_write hdrmsg OK sockfd=%d", sockfd);
				try {
       				int len2 = boost::asio::write(socket, boost::asio::buffer(zip.c_str(), zip.size()));
    				d("a00292 server write str back str=[%s] strlen=%d done sentbytes=%d sockfd=%d", zip.c_str(), zip.size(), len2, sockfd );
				} catch (std::exception& e) {
					i("E66331 error in server reply() exception [%s] clientIP_=[%s] sockfd=%d", e.what(), s(clientIP_), sockfd );
				}
            } else {
                d("E33309 error async_write() srv reply hdr len=%d sockfd=%d", len, sockfd);
            }
    });
    #endif

    int sockfd = socket_.native_handle();
    replySock( str, sockfd );
}

void OmSession::replySock( const sstr &str, int sockfd )
{
	char hdr[OMHDR_SZ+1];
	hdr[OMHDR_SZ] = '\0'; 

	//bool doCompress = false;
	OmMsgHdr mhdr(hdr, OMHDR_SZ, true);

    // int doCompressThreshold = 1000;
    ulong doCompressThreshold = 300000;

	sstr zip;
	if ( str.size() > doCompressThreshold ) {
		//doCompress = true;
		ZlibCompress::compress( str, zip);
		mhdr.setLength( zip.size() );
		mhdr.setCompressed();
	} else {
		zip = str;
		mhdr.setLength( str.size() );
		mhdr.setPlain();
	}

    safewrite( sockfd, hdr, OMHDR_SZ);
    safewrite( sockfd, zip.c_str(), zip.size() ); 

}

// Main method for handling transactions
void OmSession::doTrxnL2(const char *msg, int msglen)
{
	d("a71002 doTrxn msg.len=%d msg=[%.200s]", msglen, msg );
	sstr srvid = serv_.id_;

	OmicroTrxn t(msg);
	Byte xit = t.getXit();

	/** debug
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

	sstr peer = t.srvport_;
	sstr from = t.sender_;

	sstr err;
	int validTrxn = validateTrxn( trxnId, t, isInitTrxn, err );
	if ( validTrxn < 0 ) {
        if ( -900 == validTrxn ) {
            d("a9000 validateTrxn got -900, late doTrxnL2 can be ignored");
            // qwer need socket_.close(); ???
            return;
        }

        sstr json;
		i("E1001 error doTrxnL2 INVALID from=[%s] srvport=%s", s(t.sender_), s(serv_.srvport_)  );
		errResponse( from, srvid, "INVALID_TRXN", trxnId, err, json );
	    d("a303301 reply to endclient %s ...", s(json) );
	    reply(json, socket_);
	    d("a303301 from=%s reply to endclient %s done error validateTrxn_error=%s", s(from), s(json), s(err) );
        socket_.close();
		return;
	}

	bool rc;

	if ( isInitTrxn ) {
		d("a43713 init from=%s trxnId=[%s]", s(from), s(trxnId) );
		d("a333301  i am clientproxy (node that client connects to), initTrxn() ..." );
		if ( t.trxntype_ != OM_NEWACCT ) {
			sstr fence;
			serv_.blockMgr_.getFence( from, fence );
			t.fence_ = fence;
			d("a93910 isInitTrxn from=%s trxnid=%s  assign t.fence_=[%s]", s(from), s(trxnId), s(fence) );
		}

		rc = initTrxn( t );
		d("a333301  i am clientproxy, from=%s launched initTrxn trxnId=%s  rc=%d", s(from), s(trxnId), rc );
		sstr m;
		if ( ! rc ) {
			// okResponse( from, srvid, trxnId, "SubmitOK", m );
			errResponse( from, srvid, "INVALID_TRXN", trxnId, "initTrxn", m );
		    d("a333301 reply to endclient %s ...", s(m) );
		    reply(m, socket_);
		    d("a333301 from=%s reply to endclient %s done", s(from), s(m) );
            socket_.close();
            return;
		}

        int sockfd = socket_.native_handle();
        serv_.trxn_initer_client_socket_.emplace( trxnId, sockfd );
        // todo reply only if error, and close socket. refer to doQueryL2

	} else {
		d("a43714 from=%s exist trxnId=[%s]", s(from), s(trxnId) );
		sstr beacon = t.beacon_;
		if ( xit == XIT_i ) {
			d("a82208 from=%s %s recved XIT_i", s(from), s(sid_));
			// I got 'i', I must be a leader
			DynamicCircuit circ( serv_.nodeList_);
			strvec followers;
			bool iAmLeader = circ.isLeader( beacon, srvid, true, followers );
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
					d("a31112 from=%s %s multicast XIT_j followers for vote expect no reply ..", s(from), s(sid_));

					//i("a949444 debug: followers:" );
					//pvectag( "tagfollower", followers );

					serv_.collectKTrxn_.add(trxnId, 1);

					// may 31, 2026 t.srvport_ = serv_.srvport_;

					sstr alldat; t.allstr(alldat);
					serv_.multicast( OM_TXN,  followers, alldat, false, replyVec ); // XIT_j
					d("a31112 from=%s %s multicast XIT_j followers for vote done replyVec=%d\n", s(from), s(sid_), replyVec.size() );
					if ( followers.size() < 1 ) {
						d("a33346 no followers, onRecvK and fire next-round");
						serv_.onRecvK( beacon, trxnId, clientIP_, sid_, t );
					}

				} else {
					d("a3306 error from=%s XIT_i to state A toAgood is false trxnId=%s", s(from), s(trxnId) );
				}
			} else {
				// bad
				d("a3308 error i [%s] am not leader, from=%s ignore XIT_i, trxnId=%s", s(sid_), s(from), s(trxnId) );
			}
			// else i am not leader, igore 'i' xit
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
			d("a31112 i am follower from=%s %s multicast XIT_j followers for vote expect no reply ..", s(from), s(sid_));
			DynamicCircuit circ( serv_.nodeList_);
			strvec leader;
			bool isLeader = circ.getLeader(beacon, srvid, leader );

			//d("a10831 i am %s  my leader:", s(serv_.srvport_) );
			//pvectag("tagfollowerleader:", leader );

			if ( ! isLeader ) {
				// 6/1/2026 t.srvport_ = serv_.srvport_;
				t.setXit( XIT_k );
				strvec replyVec;
				sstr alldat; 
                t.allstr(alldat);
				d("a34112 from=%s %s unicast XIT_k from follower to leader trxnId=%s serv_.srvport_=%s ..", s(from), s(sid_), s(trxnId), s(serv_.srvport_)  );
				serv_.multicast( OM_TXN, leader, alldat, false, replyVec );  // XIT_k
			} else {
				i("E33330 error got XIT_j but i am leader, trxnId=%s serv_.srvport_=%s", s(trxnId), s(serv_.srvport_) );
			}

		} else if ( xit == XIT_k ) {
			// i am leader if reaching quorum, fire next multicast
			d("a53103 i am leader from=%s %s got XIT_k peer:[%s] trxnId=%s", s(from), s(srvid), s(peer), s(trxnId) );
			serv_.onRecvK( beacon, trxnId, clientIP_, sid_, t );
            //  if qurum reached, onRecvK does multicast( OM_TXN, otherLeaders, alldat, false, nullvec ); // XIT_l
		} else if ( xit == XIT_l ) {
			d("a92822 i am leader from=%s %s received XIT_l peer:[%s] trxnId=%s ...", s(from), s(sid_), s(peer), s(trxnId) );
			serv_.onRecvL( beacon, trxnId, clientIP_, sid_, t );
            // if quotum reached, onRecvL does multicast( OM_TXN, otherLeaders, alldat, false, nullvec ); // XIT_m
			d("a92822 from=%s %s received XIT_l peer:[%s] done trxnId=%s", s(from), s(sid_), s(peer), s(trxnId) );
		} else if ( xit == XIT_m ) {
		    // received one XIT_m, there may be more XIT_m in next 3 seconds
			d("a54103 i am leader from=%s %s got XIT_m peer:[%s] trxnId=%s", s(from), s(srvid), s(peer), s(trxnId) );
			serv_.onRecvM( beacon, trxnId, clientIP_, sid_, t );
            // blockMgr_.receiveTrxn( t )  -- leader also realize the trxn
            // if quorum reached, onRecvM does multicast( OM_TXN, followers, alldat, false, nullvec ); // XIT_n
			d("a54103 from=%s %s got XIT_m peer:[%s] done", s(from), s(srvid), s(peer) );
		} else if ( xit == XIT_n ) {
			// follower gets a trxn commit message
			Byte curState; 
			serv_.trxnState_.getState( trxnId, curState );
			if ( curState != ST_F ) {
				i("a99 follower from=[%s] commit trxnId=%s peer:[%s]", s(from), s(trxnId), s(peer) );

                sstr errmsg;
	            sstr srvid = serv_.id_;
                sstr json;
			    int nrc = serv_.onRecvN( beacon, trxnId, clientIP_, sid_, t, errmsg );
                // qwer  send Initer confirmation. initer upon receiving confirmation, it can reply back to client
				serv_.trxnState_.setState( trxnId, ST_F );  // to ST_F
                if ( 0 == nrc ) {
				    // inside onRecvN  serv_.blockMgr_.receiveTrxn( t );  // follower 
                    
                    // final reply to client
                    auto itr = serv_.trxn_initer_client_socket_.find( trxnId );
                    if ( itr != serv_.trxn_initer_client_socket_.end() ) {
                        int sockfd = itr->second;
                        sstr  stmt = "Transaction Confirmed (CONSENSUS_REACHED)";
			            okResponse( from, srvid, trxnId, stmt, json );
                        replySock( json, sockfd); 
                        ::close( sockfd );
                    }
                    serv_.trxn_initer_client_socket_.erase( trxnId );
                } else if ( nrc < 0 ) {
                    if ( nrc != -171  && nrc != -173 ) {
                        // final reply to client
                        auto itr = serv_.trxn_initer_client_socket_.find( trxnId );
                        if ( itr != serv_.trxn_initer_client_socket_.end() ) {
                            int sockfd = itr->second;
			                errResponse( from, srvid, "INVALID_TRXN", trxnId, errmsg, json );
                            replySock( json, sockfd); 
                            ::close( sockfd );
                        }
                        serv_.trxn_initer_client_socket_.erase( trxnId );
                    } else {
                        // -171 or -173 can be ignored
                    }
                }

			} else {
				d("a99991 from=%s follower no-commit curState is already ST_F peer=[%s] for trxnid=%s", s(from), s(peer), s(trxnId) );
			}
		} else {
			d("a200 error doTrxnL2 from=[%s] xit is unknown [%c] peer:%s trxnid=%s ", s(from), xit, s(serv_.srvport_), s(trxnId)  );
		}
    }

	d("a1000 doTrxnL2 from=[%s] trxntype=[%s] my srvport=%s peer=%s xit=%c txnid=%s isInitTrxn=%d trxnid=%s Done\n", 
		s(t.sender_), s(t.trxntype_), s(serv_.srvport_), s(t.srvport_), xit, s(trxnId), isInitTrxn, s(trxnId) );
}

// Handles simple queris
void OmSession::doSimpleQuery(const char *msg, int msglen)
{
	d("a71004 doSimpleQuery msg.len=%d msg=[%.200ss]", msglen, msg );
	sstr srvid = serv_.id_;

    rapidjson::Document dom;
    dom.Parse( msg, msglen );

    if ( dom.HasParseError() ) {
        i("E43337 error dom.HasParseError msg=[%.200s] INVALID_QUERY\n", msg );
		sstr m;
		errResponse( "", srvid, "INVALID_QUERY_PARSE_ERROR", "notrxnid", msg, m );
		reply( m, socket_ ); 
        return;
    }

    sstr qtype = dom["QT"].GetString();
    sstr trxnId = dom["TID"].GetString();
    sstr sender = dom["FRM"].GetString();
    sstr ts = dom["TS"].GetString();

	if ( qtype == "QP" ) {
		// request public key
        d("a70112 client request public key, now send serv_.pubKey_ back to client ...");
		sstr json;
		okResponse( sender, srvid, trxnId, serv_.pubKey_, json);
		reply( json, socket_ ); 
        d("a70112 client request public key, now send serv_.pubKey_ back to client is done");
	} else if ( qtype == "QT" ) {
		// query trxn status
        d("a711102 cleint query trxn status ...");
		sstr json;
		serv_.blockMgr_.queryTrxn( sender, trxnId, ts, json );
		reply( json, socket_ ); 
		d("a40088 received QT return res=[%s] replt back to cleint is done", s(json));
	} else if ( qtype == "QQ" ) {
		d("a2939 QQ cleint request last query result ...");
		// request last query result
		uint nodeLen = serv_.nodeList_.size();

        // wait for condvar

		// int votes = serv_.collectQueryKTrxn_.get(trxnId);
		int votes = serv_.collectQueryLTrxn_.get(trxnId);
		d("a32209 read collectQueryLTrx nodeLen=%d votes=%d trxnId=%s", nodeLen, votes, s(trxnId) );
		if ( uint(votes) < (2*nodeLen/3) ) {
			d("a040449 query not enough votes");
			sstr m;
			errResponse( sender, srvid, "NOTFOUND", trxnId, "NOT_ENOUGH_VOTES", m );
			reply( m, socket_ ); 
		} else {
			d("a040443 query got enough votes");
			sstr lastResult;
			getQueryResult( trxnId, sender, lastResult );
			reply( lastResult, socket_ ); 
		}
	} else {
		sstr m;
		errResponse( sender, srvid, "INVALID_REQUEST", trxnId, msg, m );
		reply( m, socket_ ); 
	}

	d("a535023 doSimpleQuery done clientIP_=[%s]", s(clientIP_));
}

// Initialize a transaction from client
bool OmSession::initTrxn( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	sstr beacon = txn.beacon_;
	sstr trxnid; 
    txn.getTrxnID( trxnid );
	d("a80123 initTrxn() threadid=%ld beacon=[%s] trxnid=[%s]", pthread_self(), s(beacon), s(trxnid) );

	// for each zone leader
	//   send leader msg: trxn, with tranit XIT_i
	// self node maybe one of the zone leaders
	DynamicCircuit circ(serv_.nodeList_);
	strvec hostVec;
	circ.getZoneLeaders( beacon, hostVec );

	/***
	for ( auto &id: hostVec ) {
		d("a20112 initTrxn send XIT_i to leader [%s]", s(id) );
		strvec followers;
		bool rc =  circ.isLeader( beacon, id, true, followers  );
		//d("  %d leader %s has followers:", rc, s(id) );
		//pvectag("   tagfollower:", followers );
	}
	***/

	txn.setNotInitTrxn();
	txn.setXit( XIT_i );

	strvec replyVec;
	d("a31181 multicast to ZoneLeaders expectReply=false txn.srvport_=%s ...", s(serv_.srvport_) );
	//pvec( hostVec );

	// may 31, 2026 txn.srvport_ = serv_.srvport_;

	sstr alldat; 
    txn.allstr(alldat);

    d("a011929 multicast alldat=[%s] ZoneLeaders hostVec.size=%d ...", s(alldat), hostVec.size() );

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

void OmSession::makeSessionID()
{
    struct timeval now;
    gettimeofday( &now, NULL );
	char buf[16];
	sprintf(buf, "%d%ld", int(now.tv_sec%10),  now.tv_usec);
	sid_ = buf;
}

// Check if a received transaction is valid.
// It checks multiple conditions:  state, identity, signature, balance, double-spending.
// bool OmSession::validateTrxn( const sstr &trxnId,  OmicroTrxn &txn, bool isInitTrxn, sstr &err )
// return 0 for OK; < 0 for error; -900 for ignore (late commers)
int OmSession::validateTrxn( const sstr &trxnId,  OmicroTrxn &txn, bool isInitTrxn, sstr &err )
{
	d("a17621 OmSession::validateTrxn serv_.address=[%s] serv_.port=[%s]", s(serv_.address_), s(serv_.port_) );
	//d("22206 my serv_.pubKey_=[%s]", s(serv_.pubKey_) );
	//d("22206 my serv_.secKey_=[%s]", s(serv_.secKey_) );

	// check state if ST_F then all done, ignore trxn
	Byte curState;
	bool rc = serv_.trxnState_.getState( trxnId, curState );
	if ( curState >= ST_F ) {
		d("E30280 trxn is done in ST_F state.trxnid=[%s] curState(%u) >= ST_F(%d) rc=%d", s(txn.sender_), s(trxnId), curState, ST_F, rc  );
		err = sstr("Late trxn request ") + serv_.address_ + ":" + serv_.port_;
		// return false;
		return -900;
	}

	bool validTrxn = txn.validateTrxn( serv_.secKey_ );
	if ( ! validTrxn ) {
		i("E30290 trxn is invalid. I am node: %s  trxnid=[%s]", s(serv_.srvport_), s(trxnId)  );
		i("E30290 trxn invalid. sender node: %s", s(txn.srvport_) );
		err = sstr("Transaction object invalid ") + serv_.address_ + ":" + serv_.port_;
		// return false;
		return -10;
	}

	if ( txn.trxntype_ == OM_PAYMENT ) {
		double bal;
		sstr pubkey;
		int rc = serv_.blockMgr_.getBalanceAndPubkey( txn.sender_, bal, pubkey );
		if ( rc < 0 ) {
            i("E32018 error from=[%s] invalid rc=%d", s(txn.sender_), rc );
			err = sstr("Unable to get balance and public key of sender [") + txn.sender_ + "] in PAYMENT";
            // return false;
            return -20;
		}

		if ( pubkey != txn.userPubkey_ ) {
            i("E32016 pubkey mismatch from=[%s] pubkey ", s(txn.sender_) );
            i("E32016 txn.pubkey=[%s]", s(txn.userPubkey_) );
            i("E32016 acct.pubkey=[%s]", s(pubkey) );
			err = "Public key mismatch";
            // return false;
            return -30;
		}

    	double amt = txn.getAmountDouble();
        d("a44502 from=[%s] balance=[%.6g]", s(txn.sender_), bal );
        if ( (bal - amt) < 0.0001 ) {
            d("a30138 from=[%s] balance=%.6g not enough to pay %.6g", s(txn.sender_), bal, amt );
			err = "Not enough funds";
            // return false;
            return -40;
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
					// return false;
					return -50;
				} else {
					d("a900123 fencing diff, but blockMgr_.readTrxns not found brc=%d, go ahead errmsg=%s", s(errmsg));
				}
			}
		}
	} else if ( txn.trxntype_ == OM_XFERTOKEN ) {
        sstr errmsg;
		int rc = serv_.blockMgr_.isXferTokenValid( txn, errmsg );
		if ( rc < 0 ) {
           	i("E35012 from=[%s] xfer token invalid ", s(txn.sender_), s(txn.receiver_)  );
			err = sstr("Transfer error: xfer token invalid. ") + errmsg ;
			return -60;
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
					return -70;
				} else {
					d("a900124 fencing diff, but blockMgr_.readTrxns not found brc=%d, go ahead errmsg=%s", s(errmsg));
				}
			}
		}
	}

	return 0;
}

// It checks validity of a query,such as balance, tokens
bool OmSession::validateQuery( OmicroTrxn &txn, const sstr &trxnId, bool isInitTrxn, sstr &err )
{
	d("a82220 enter OmSession::validateQuery");
	//d("22208 my serv_.pubKey_=[%s]", s(serv_.pubKey_) );
	//d("22208 my serv_.secKey_=[%s]", s(serv_.secKey_) );

	bool validTrxn = txn.validateTrxn( serv_.secKey_ );
	if ( ! validTrxn ) {
		i("E20290 validateQuery query is not valid");
		err = "Invalid tranaction message";
		return false;
	}

	// tentatively check if trxn would be valid: perform query get result
    d("a930348 inside validateQuery() now serv_.blockMgr_.runQuery ...");

	sstr res;
	int rc = serv_.blockMgr_.runQuery(txn, res);
	if  ( rc < 0 ) {
		i("E40021 runQuery error rc=%d  request=[%s]", rc, s(txn.request_) );
		err = "Query error";
		return false;
	}

	if ( isInitTrxn ) {
		txn.response_ = res;
		d("a38800 after runQuery OK for isInitTrxn=%d", isInitTrxn );
		return true;
	}

	if ( txn.response_ != res ) {
		d("a31800 error after runQuery  txn.response_ != res ");
		d("a1004 txn.response_=[%s]", s(txn.response_) );
		d("a1004 res =[%s]", s(res) );
		err = "Query result mismatch";
		return false;
	} else {
		d("a31400 after temp runQuery  txn.response_ == res OK ");
		serv_.qResult_.add(trxnId, res);
		d("a31400 after temp runQuery sid_=[%s]  trxnId=[%s] res=[%s] res is added to serv_.qResult_ for the trxnId", s(sid_),  s(trxnId), s(res) );
		return true;
	}

}

// Main method for running a query, verified and voted by all nodes.
// It is similar to trxn, but shorter.
void OmSession::doQueryL2(const char *msg, int msglen)
{
	d("a71003 doQueryL2 msg.len=%d msg=[%.257s]", msglen, msg );
	sstr srvid = serv_.id_;

    int sockfd = socket_.native_handle();
    d("a55539 in doQueryL2 ctor socket_.native_fd sockfd=%d", sockfd);

	OmicroTrxn t(msg);

	/** debug
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

    Byte xit = t.getXit();
    d("a000128 doQueryL2() trxnId=%s  xit=%c", s(trxnId), xit );


	sstr err;
	bool validTrxn = validateQuery( t, trxnId, isInitTrxn, err );
    d("a8112003 in doQueryL2() validateQuery return validTrxn=%d", validTrxn );

	// run query in validateQuery
	// if ( ! validTrxn && xit != 'l' ) 
	if ( ! validTrxn ) {
		i("E40201 error INVALID_TRXN query ignore trxnId=%s sening INVALID_QUERY back to client", s(trxnId) );
		sstr json;
		errResponse( t.sender_, srvid, "INVALID_QUERY_INVALID_TRXN", trxnId, srvid + "|" + err, json );
		reply(json, socket_);
		i("E40202 error INVALID_TRXN query ignore trxnId=%s", s(trxnId) );
		return;
	}

	bool rc;
	sstr m;

    /**
	sstr peer = t.srvport_;
    d("a039930 peer=%s", s(peer) );
    **/

	if ( isInitTrxn ) {
		d("a41713 in doQueryL2() initquery trxnId=[%s]", s(trxnId) );

        // send XIT_i to all leaders
		rc = initQuery( t );

        sstr initerIP = t.getIniterIP();
        sstr initerPort = t.getIniterPort();
        d("a711120 in doQueryL2 isInitTrxn=true  trxnt.initerIP=%s initerPort=%s", s(initerIP), s(initerPort) );

		OmResponse resp;
		resp.TID_ = trxnId;
		if ( ! rc ) {
			resp.STT_ = OM_RESP_ERR;
			resp.RSN_ = "INVALID_QUERY:INITQUERY_ERROR";
		    i("E40205 initQuery failed, INVALID_TRXN query ignore trxnId=%s socket_.close()", s(trxnId) );
		    resp.json( m );
		    reply(m, socket_);
			socket_.close();
            return;
		}

        int sockfd = socket_.native_handle();
        serv_.query_initer_client_socket_.emplace( trxnId, sockfd );

        // debug
        d("a030394 todo reply back some info back to client native sockfd=%d", sockfd );
        /***
		resp.json( m );
		reply(m, socket_);
        ***/



	} else {
		// Byte xit = t.getXit();
		d("a43712 existing trxnId=[%s] xit=[%c]", s(trxnId), xit );
		sstr beacon = t.beacon_;

        sstr initerIP = t.getIniterIP();
        sstr initerPort = t.getIniterPort();
        d("a711120 in doQueryL2 isInitTrxn=false  trxnt.initerIP=%s initerPort=%s", s(initerIP), s(initerPort) );


		if ( xit == XIT_i ) {
            // i am leader, i will send XIT_j to all my followers

			d("a84213 %s recved XIT_i", s(sid_));
			// I got 'i', I must be a leader
			DynamicCircuit circ( serv_.nodeList_);
			strvec followers;
			bool iAmLeader = circ.isLeader( beacon, srvid, true, followers );
            d("a50505929 iAmLeader=%d", iAmLeader );

            // todo debug
            for ( auto fip : followers ) {
                d("a2455509 todo follower ip=%s", s(fip) );
            }


			if ( iAmLeader ) {
				// state to A
				bool toAgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_i );
				if ( toAgood ) {
					d("a01233 XIT_i toAgood ST_A true trxnid=[%s]", s(trxnId));

					// send XIT_j to all followers in this leader zone
					bool toBgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_j );
					d("a22200 i am Leader XIT_j  toBgood=%d trxnid=[%s]", toBgood, s(trxnId) );

					t.setXit( XIT_j );
					strvec replyVec;
					d("a32112 %s multicast XIT_j followers for vote expect reply ..", s(sid_));

					//i("a949449 debug: followers:" );
					//pvectag( "tagfollower", followers );

					// 6/1/2026 serv_.collectQueryKTrxn_.add(trxnId, 1);
                    // d("a03939 collectQueryKTrxn_.add trxnId=%s 1", s( trxnId ) );

                    // sstr oldsrvport = t.srvport_;
					// may 31, 2026  t.srvport_ = serv_.srvport_;

					sstr alldata; t.allstr(alldata);
                    // d("a81129 reset t.srvport_ from %s to %s", s(oldsrvport), s(t.srvport_ ) );

					serv_.multicast( OM_XNQ, followers, alldata, false, replyVec ); // sending XIT_j to followers
					d("a32112 %s multicast XIT_j followers for vote done replyVec=%d\n", s(sid_), replyVec.size() );
					if ( followers.size() < 1 ) {
						d("a422201 no followers, leader check onRecvQueryK and fire off next round");
						serv_.onRecvQueryK( beacon, trxnId, clientIP_, sid_, t );
					}
				} else {
					d("a3306 in doQueryL2 XIT_i to state A toAgood is false");
				}
			} else {
				// bad
				d("a3308 i [%s] am not leader, in doQueryL2, ignore XIT_i", s(sid_));
			}
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
            d("a200883 idoQueryL2 receved XIT_j ");
			DynamicCircuit circ( serv_.nodeList_);
			strvec leader;
			strvec replyVec;

			bool iAmLeader = circ.getLeader( beacon, srvid, leader );
			if ( ! iAmLeader ) {

				t.setXit( XIT_k );

                d("a555508 upon recving XIT_j, i am not leader, and send my vote XIT_k to leader:");
                // todo debug
                for  ( auto ld : leader ) {
                    d("a220038 i am follower, i now send XIT_k to leader %s", s(ld) );
                }

                // sstr oldsrvport = t.srvport_;
				// may 31, 2026 t.srvport_ = serv_.srvport_;
				sstr alldata; t.allstr(alldata);

                //d("a81120 reset t.srvport_ from %s to %s", s(oldsrvport), s(t.srvport_ ) );

				serv_.multicast( OM_XNQ, leader, alldata, false, replyVec ); // sending XIT_k to my leader
			} else {
                d("a555508 upon recving XIT_j, i am leader, and do nothing");
            }
		} else if ( xit == XIT_k ) {
            d("a200885 in doQueryL2 i am leader receved XIT_k call serv_.onRecvQueryK ");
			int krc = serv_.onRecvQueryK( beacon, trxnId, clientIP_, sid_, t );
            if ( 0 == krc ) {
                // got enough XIT_k votes
                sstr initerIP = t.getIniterIP();
                sstr initerPort = t.getIniterPort();

                uint portn = static_cast<unsigned int>( std::stoul(initerPort));

                if ( initerIP == myIP_ && portn == myPort_ ) {
                    d("a50058 i am leader and i am the initer, do not send XIT_l to myself");
                } else {
                    strvec initer;
    			    strvec replyVec;

                    d("a2022390 serv_.id_=[%s] initerIP=[%s]  initerPort=[%s]", s(serv_.id_), s(initerIP), s(initerPort) );
    
                    // sstr hostrec = serv_.id_ + "|" + initerIP + "|" + initerPort;
                    sstr hostrec = sstr("0|") + initerIP + "|" + initerPort;
                    d("a6001274 hostrec=%s", s(hostrec) );
                    initer.push_back( hostrec );
    
                    t.setXit( XIT_l );
    				sstr alldata; t.allstr(alldata);
    
                    
                    d("a300048 send XIT_l to initer %s %s ...", s(initerIP), s(initerPort) );
    				serv_.multicast( OM_XNQ, initer, alldata, false, replyVec ); // sending XIT_l to proxy(initer) node
                    d("a300048 send XIT_l to initer %s %s done", s(initerIP), s(initerPort) );
                }
                
            }
		} else if ( xit == XIT_l ) {
            d("a203885 initer in doQueryL2 receved XIT_l call serv_.onRecvQueryL ");
			int lrc = serv_.onRecvQueryL( beacon, trxnId, clientIP_, sid_, t );
            d("a203885 in doQueryL2 initer receved XIT_l call serv_.onRecvQueryL done lrc=%d", lrc );
            if ( 0 == lrc ) {
                // prepare data for QQ
                // qwer
                auto itr = serv_.query_initer_client_socket_.find( trxnId );
                if ( itr != serv_.query_initer_client_socket_.end() ) {
                    int sockfd = itr->second;

    			    sstr lastResult;
    			    getQueryResult( trxnId, t.sender_, lastResult );
                    //bool crc = initSocket.is_open();
                    //int sockfd = initSocket.native_handle();
                    d("a9992039 getQueryResult lastResult=[%s] reply to client socket_ native sockfd=%d  ...", s(lastResult), sockfd );
    			    replySock( lastResult, sockfd ); 
                    serv_.query_initer_client_socket_.erase( trxnId );
                    //d("a0088448 reply is done sz=%ld", sz );
                    d("a0088448 replySock is done" );
                } else {
                    d("a02923948 error serv_.query_initer_client_socket_ has no trxnId, no send result initer");
                }
            }
		} else {
            d("a992830 in doQueryL2 xit is unknown %d", xit );
        }
    }

	d("a555024 doQueryL2 done clientIP_=[%s] trxnId=%s", s(clientIP_), s(trxnId) );
}  // doQueryL2

// Initiate a query process
// send XIT_i to all leaders
bool OmSession::initQuery( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	sstr beacon = txn.beacon_;
	sstr trxnid; 
    txn.getTrxnID( trxnid );
	d("a80120 tQueryinitTrxn() threadid=%ld beacon=[%s] trxnid=[%s]", pthread_self(), s(beacon), s(trxnid) );

	// for each zone leader
	//   send leader msg: trxn, with tranit XIT_i
	// self node maybe one of the zone leaders
	DynamicCircuit circ(serv_.nodeList_);
	strvec leaderVec;
	circ.getZoneLeaders( beacon, leaderVec );

    // todo debug
	for ( auto &id: leaderVec ) {
		d("a20122 in initQuery will send XIT_i to leader [%s]", s(id) );
	}

	txn.setNotInitTrxn();
	txn.setXit( XIT_i );

	txn.srvport_ = serv_.srvport_;  // query initiator IP and port
    d("a000123 initQuery() txn.srvport_=%s", s(txn.srvport_ ) );

	strvec replyVec;
	d("a31182 multicast to ZoneLeaders expectReply=false ...");
	//pvec( hostVec );

	int connected;
	uint twofp1;
	bool expectReply = false;

	sstr alldat; 
    txn.allstr(alldat);

    d("a203982 multicast OM_XNQ expectReply=false ...");
	connected = serv_.multicast( OM_XNQ, leaderVec, alldat, expectReply, replyVec );
    d("a203982 multicast done");

	if ( expectReply ) {
		twofp1 = twofplus1(replyVec.size());
	} else {
		twofp1 = twofplus1(leaderVec.size());
	}

	d("a31174 multicast to ZoneLeaders done connected=%d twofp1=%u", connected, twofp1);

	if ( uint(connected) >= twofp1 ) {
		d("a4321 connected=%u >= onefp1=%u true", uint(connected), twofp1 );
		return true;
	} else {
		d("a4322 connected=%u < onefp1=%u false", uint(connected), twofp1 );
		return false;
	}
}

// Get query result
// res is json retured
void OmSession::getQueryResult( const sstr &trxnId, const sstr &sender, sstr &res )
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

