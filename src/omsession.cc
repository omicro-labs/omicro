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
#include "omstrsplit.h"

EXTERN_LOGGING
using namespace boost::asio::ip;

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
				char *data = (char*)malloc( dlen );
				data[dlen] = '\0';
				d("a63003 a91838 srv doread dlen=%d length=%d hdr_[%s]", dlen, length, hdr_);

				bcode ec2;
				int len2 =  boost::asio::read( socket_, boost::asio::buffer(data,dlen), ec2 );
				d("a45023 boost::asio::read len2=%d dlen=%d", len2, dlen );
				data[len2] = '\0';

				char t = mhdr.getMsgType();
				if ( t == OM_RX ) {
					doTrxn( data, len2 );
				} else if ( t == OM_RQ ) {
					doQuery( data, len2 );
				} else {
					d("E30292 error invalid msgtype [%c]", t );
				}

				free(data);
    			do_read();
            } else {
                d("a82838 srv do read read no data");
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
            d("a51550 111 in omsession::reply str=[%s] len=%d", s(str), str.size() );
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

void omsession::doTrxn(const char *msg, int msglen)
{
	//d("a71002 doTrxn msg.len=%d msg=[%s]", msg.size(), s(msg) );
	d("a71002 doTrxn msg.len=%d", msglen );
	sstr id_ = serv_.id_;

	OmicroTrxn t(msg);

	bool validTrxn = t.isValidClientTrxn( serv_.secKey_);
	if ( ! validTrxn ) {
		sstr m = sstr("BAD_INVALID_TRXN|") + id_;
		reply(m, socket_);
		i("E40282 BAD_INVALID_TRXN ignore" );
		return;
	}

	sstr trxnId; 
	bool isInitTrxn = t.isInitTrxn();
	bool rc;
	sstr pfrom = t.srvport;

	if ( isInitTrxn ) {
		t.setID();
		t.getTrxnID( trxnId );
		d("a43713 init trxnId=[%s]", s(trxnId) );
		sstr m;
		d("a333301  i am clientnode, launching initTrxn ..." );
		rc = initTrxn( t );
		d("a333301  i am clientnode, launched initTrxn rc=%d", rc );
		if ( rc ) {
			m = sstr("GOOD_TRXN|initTrxn|") +trxnId + "|" + sid_;
		} else {
			m = sstr("BAD_TRXN|initTrxn|") +trxnId + "|" + sid_;
		}

		d("a333301 reply to endclient %s", s(m) );
		reply(m, socket_);
		d("a333301 reply to endclient %s done", s(m) );
	} else {
		t.getTrxnID( trxnId );
		d("a43714 exist trxnId=[%s]", s(trxnId) );
		Byte xit = t.getXit();
		sstr beacon = t.beacon;
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
					t.srvport = serv_.srvport_;
					omserver::multicast( followers, t.str(), true, replyVec );
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
							omserver::multicast( otherLeaders, t.str(), false, replyVec );
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
			sstr m = sstr("GOOD_TRXN|XIT_j|")+id_ + "|" + sid_;;
			d("a555550 GOOD_TRXN|XIT_j m=[%s]", m.c_str() );
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
			serv_.blockMgr_.saveTrxn( t );
		} else if ( xit == XIT_z ) {
			// query trxn status
			sstr res;
			serv_.blockMgr_.queryTrxn( trxnId, res );
			reply( res, socket_ ); 
			d("a40088 received XIT_z return res");
		}
    }

	d("a555023 doTrxn done clientIP_=[%s]", s(clientIP_));
}

void omsession::doQuery(const char *msg, int msglen)
{
	d("a71002 doQuery msg.len=%d msg=[%s]", msglen, msg );
	sstr id_ = serv_.id_;

	OmStrSplit sp( msg, '|');
	sstr qtype = sp[0]; 
	sstr trxnId = sp[1]; 

	if ( qtype == "QT" ) {
		// query trxn status
		sstr res;
		serv_.blockMgr_.queryTrxn( trxnId, res );
		reply( res, socket_ ); 
		d("a40088 received QT return res");
	} else if ( qtype == "QP" ) {
		// request public key
		reply( serv_.pubKey_, socket_ ); 
	} else {
		reply( sstr(msg) + "|BADREQUEST", socket_ ); 
	}

	d("a535023 doQuery done clientIP_=[%s]", s(clientIP_));
}

bool omsession::initTrxn( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// serv_.nodeList_ is std::vector<string>
	sstr beacon = txn.beacon;
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
	txn.srvport = serv_.srvport_;
	int connected = omserver::multicast( hostVec, txn.str(), false, replyVec );
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

