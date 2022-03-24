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

EXTERN_LOGGING
using namespace boost::asio::ip;

omsession::omsession(boost::asio::io_context& io_context, omserver &srv, tcp::socket socket)
        : io_context_(io_context), serv_(srv), socket_(std::move(socket))
{
    stop_ = false;
	clientIP_ = socket_.remote_endpoint().address().to_string();
	makeSessionID();
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
				OmMsgHdr mhdr(hdr_, OMHDR_SZ);
				ulong dlen = mhdr.getLength();
				char *data = (char*)malloc( dlen );
				d("a63003 a91838 srv doread dlen=%d", dlen);

				boost::asio::async_read(socket_, boost::asio::buffer(data,dlen), 
				  [this, self, data](bcode ec2, std::size_t length2 )
				{
                    d("a82828 srv do read read data[%d]", length2 );
                    sstr msg(data, length2);
				    free(data);
					try {
                        callback( msg );
					} catch ( std::exception& e ) {
						i("a92201 in async_read data exception: [%s]", e.what() );
						abort();
					    return;
					}
    				do_read();
				});
            } else {
                d("a82838 srv do read read no data");
			}
    });
}

#if 0
void omsession::do_write(std::size_t length)
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
          [this, self](bcode ec, std::size_t /*length*/)
          {
            if (!ec)  // no error, OK
            {
                // do_read();
                d("a82823 write_some data_[%s]", data_ );
            }
    });
}
#endif

void omsession::reply( const sstr &str )
{
	OmMsgHdr mhdr(hdr_, OMHDR_SZ);
	mhdr.setLength( str.size() );
	mhdr.setPlain();

    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(hdr_, OMHDR_SZ),
          [this, self, str](bcode ec, std::size_t len)
          {
            d("a55550 111 in omsession::reply str=[%s] len=%d", s(str), str.size() );
            if (!ec)  // no error, OK
            {
                d("a33300 srv replyback hdr len=%d", len );
                boost::asio::async_write(socket_, boost::asio::buffer(str.c_str(), str.size() ),
                    [this, self, str](bcode ec, std::size_t len2)
                    {
                        d("a82823 222 srver reply back, async_write str=[%s] srlen=%d len2=%d", s(str), str.size(), len2 );
                    });
            } else {
                d("a33309 111 error srv reply hdr len=%d", len);
            }
    });

	/***
	try {
    	d("a53550 in omsession::reply str=[%s] len=%d", s(str), str.size() );
    	d("a00291 server write header back hdr_=[%s] ...", hdr_ );
        int len1 = boost::asio::write(socket_, boost::asio::buffer(hdr_, OMHDR_SZ) );
    	d("a00291 server write header back hdr_=[%s] done sentbytes=%d", hdr_, len1 );
    
    	d("a00292 server write str back str=[%s] ...", str.c_str() );
       	int len2 = boost::asio::write(socket_, boost::asio::buffer(str.c_str(), str.size()));
    	d("a00292 server write str back str=[%s] strlen=%d done sentbytes=%d", str.c_str(), str.size(), len2 );
	} catch (std::exception& e) {
		i("a66331 in server reply() exception [%s] clientIP_=[%s]", e.what(), s(clientIP_) );
		abort();
	}
	***/

}

void omsession::callback(const sstr &msg)
{
	sstr id_ = serv_.id_;

	d("a2108 callback: msg=[%s]", msg.substr(0, 40).c_str());

	// auto send back msg for testing.
	/**
	sstr m1 = sstr("Server echo back: hello from server ") + id_;
	d("a0223848 Server echo bac m1.size=%d", m1.size() );
	reply( m1 );
	**/

	OmicroTrxn t(msg.c_str());
	// t.setSrvPort( serv_.srvport_.c_str() );

	bool validTrxn = t.isValidClientTrxn();
	if ( ! validTrxn ) {
		sstr m(sstr("BAD_INVALID_TRXN|") + id_ + "|" + msg);
		reply(m);
		return;
	}

	sstr trxnId = t.getTrxnID();
	bool isInitTrxn = t.isInitTrxn();
	bool rc;
	char *pfrom = t.getSrvPort();

	if ( isInitTrxn ) {
		sstr m;
		d("a333301  i am any node, launching initTrxn ..." );
		rc = initTrxn( t );
		d("a333301  i am any node, launched initTrxn rc=%d", rc );
		if ( rc ) {
			m = sstr("GOOD_TRXN|") +id_;
		} else {
			m = sstr("BAD_TRXN|") +id_ + "|" + msg;
		}
		d("a333301 reply %s", s(m) );
		reply( m);
		d("a333301 reply %s done", s(m) );
	} else {
		Byte xit = t.getXit();
		sstr beacon = t.getBeacon();
		if ( xit == XIT_i ) {
			d("a82208 %s recved XIT_i", s(id_));
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
					d("a31112 %s multicast XIT_j followers for vote expect reply ..", s(id_));
					pvec( followers );
					t.setSrvPort( serv_.srvport_.c_str() );
					omserver::multicast( followers, t.str(), true, replyVec );
					d("a31112 %s multicast XIT_j followers for vote done replyVec=%d\n", s(id_), replyVec.size() );

					// got replies from followers, state to C
					bool toCgood = serv_.trxnState_.goState( serv_.level_, trxnId, XIT_k );
					if ( toCgood ) {
						d("a55550 recv XIT_k toCgood true");
						if ( serv_.level_ == 2 ) {
							int votes = replyVec.size(); // how many replied
							t.setVoteInt( votes );
							// send el-'l' xit to other leaders
							t.setXit( XIT_l );
							strvec otherLeaders;
							circ.getOtherLeaders( beacon, id_, otherLeaders );
							d("a31102 %s round-1 multicast XIT_l otherLeaders noreplyexpected ..", s(id_));
							pvec(otherLeaders);
							// txn.setSrvPort( serv_.srvport_.c_str() );
							omserver::multicast( otherLeaders, t.str(), false, replyVec );
							d("a31102 %s round-1 multicast XIT_l otherLeaders done replyVec=%d\n", s(id_), replyVec.size() );
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
				d("a3308 i [%s] am not leader, ignore XIT_i", s(id_));
			}
			// else i am not leader, igore 'i' xit
		} else if ( xit == XIT_j ) {
			// I am follower, give my vote to leader
			d("a5501 received XIT_j from [%s] reply back good", pfrom);
			sstr m = sstr("GOOD_TRXN|XIT_j|")+id_;
			reply(m);
		} else if ( xit == XIT_l ) {
			d("a92822 %s received XIT_l from [%s] ...", s(id_), pfrom );
			serv_.onRecvL( beacon, trxnId, clientIP_, sid_, t );
		} else if ( xit == XIT_m ) {
		    // received one XIT_m, there may be more XIT_m in next 3 seconds
			// qwer
			d("a54103 %s got XIT_m from [%s]", s(id_), pfrom );
			serv_.onRecvM( beacon, trxnId, clientIP_, sid_, t );
		} else if ( xit == XIT_n ) {
			// follower gets a trxn commit message
			d("a9999 follower commit a TRXN %s from [%s]", s(trxnId), pfrom);
		}
    }

	free(pfrom);
	d("a555023 callback done clientIP_=[%s]", s(clientIP_));
}

bool omsession::initTrxn( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// serv_.nodeList_ is std::vector<string>
	sstr beacon = txn.getBeacon();
	sstr trxnid = txn.getTrxnID();
	d("a80123 initTrxn() threadid=%ld beacon=[%s] trxnid=%s", pthread_self(), s(beacon), trxnid.substr(0,10).c_str() );

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
	d("a3118 multicast to ZoneLeaders ...");
	pvec( hostVec );
	txn.setSrvPort( serv_.srvport_.c_str() );
	omserver::multicast( hostVec, txn.str(), true, replyVec );
	d("a3118 multicast to ZoneLeaders done");

	uint numReply = replyVec.size();
	uint onefp1 = onefplus1(hostVec.size());
	d("a4550 numReply=%d onefp1=%d", numReply, onefp1);
	if ( numReply >= onefp1 ) {
		return true;
	} else {
		return false;
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

