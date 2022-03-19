#include <boost/bind.hpp>
#include <signal.h>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <malloc.h>

#include <memory>
#include <utility>
#include <boost/asio.hpp>

#include "omicrotrxn.h"
#include "server.hpp"
#include "dynamiccircuit.h"
#include "omutil.h"
#include "omicroclient.h"
#include "ommsghdr.h"

EXTERN_LOGGING
using namespace boost::asio::ip;

omsession::omsession(sstr id, int level, const NodeList &nodeL, tcp::socket socket)
        : socket_(std::move(socket)), nodeList_(nodeL)
{
    stop_ = false;
    id_ = id;
    level_ = level;
    memset(data_, 0, max_length);
}

void omsession::start()
{
    do_read();
}

/***
void omsession::do_read()
{
    auto self(shared_from_this());

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
          [this, self](boost::system::error_code ec, std::size_t length)
          {
            // hanlder after reading data into data_
            if (!ec)  // no error
            {
                d("a82828 read_some data_[%d]", length );
				std::cout << "a93838 socket_.async_read_some " << length << std::endl;
                sstr msg(data_, length);
                callback( msg );
				do_read();
            } else {
                d("a82838 read no data");
				std::cout << "a93838 read no data" << std::endl;
			}
    });
}
**/

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
    		    std::cout << "a91838 dlen=" << dlen << std::endl;

				boost::asio::async_read(socket_, boost::asio::buffer(data, dlen),
				  [this, self, data](bcode ec2, std::size_t length2 )
				  {
                      d("a82828 read data[%d]", length2 );
    				  std::cout << "a93838 socket_.async_read " << length2 << std::endl;
                      sstr msg(data, length2);
                      callback( msg );
    				  do_read();
				  });
				  free(data);
            } else {
                d("a82838 read no data");
				std::cout << "a93838 read no data" << std::endl;
			}
    });
}

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

void omsession::reply( const sstr &str )
{
	OmMsgHdr mhdr(hdr_, OMHDR_SZ);
	mhdr.setLength( str.size() );
	mhdr.setPlain();

    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(hdr_, OMHDR_SZ),
          [this, self, &str](bcode ec, std::size_t)
          {
            if (!ec)  // no error, OK
            {
    			boost::asio::async_write(socket_, boost::asio::buffer(str.c_str(), str.size() ),
				    [this, self, &str](bcode ec, std::size_t)
					{
                		d("a82823 reply [%s]", s(str) );
					});
            }
    });
}

void omsession::callback(const sstr &msg)
{
	d("a2108 callback: msg=[%s]", msg.substr(0, 20).c_str());

    {
        // auto send back msg for testing.
		sstr m1 = sstr("Server echo back: hello from server " + id_);
		reply( m1 );

		OmicroTrxn t(msg.c_str());
		bool validTrxn = t.isValidClientTrxn();
		if ( ! validTrxn ) {
			sstr m("BAD_INVALID_TRXN|" + id_);
			reply(m);
			return;
		}

		sstr trxnId = t.getTrxnID();
		bool isInitTrxn = t.isInitTrxn();
		bool rc;
		if ( isInitTrxn ) {
			sstr m;
			rc = initTrxn( t );
			if ( rc ) {
				m = sstr("GOOD_TRXN|" +id_);
			} else {
				m = sstr("BAD_TRXN|" +id_);
			}
			reply( m);
		} else {
			Byte xit = t.getXit();
			sstr beacon = t.getBeacon();
			if ( xit == XIT_i ) {
				// I got 'i', I must be a leader
				DynamicCircuit circ( nodeList_);
				strvec followers;
				bool iAmLeader = circ.isLeader( beacon, id_, true, followers );
				if ( iAmLeader ) {
					// state to A
					bool goodXit = trxnState_.goState( level_, trxnId, XIT_i );
					if ( goodXit ) {
						// send XIT_j to all followers in this leader zone
						bool toBgood = trxnState_.goState( level_, trxnId, XIT_j );
						d("a21200 iAmLeader toBgood=%d", toBgood );

						t.setXit( XIT_j );
						strvec replyVec;
						d("a31112 %s multicast followers for vote ..", s(id_));
						pvec( followers );
						multicast( followers, t.str(), true, replyVec );
						d("a31112 %s multicast followers for vote done", s(id_));

						// got replies from followers, state to C
						bool toCgood = trxnState_.goState( level_, trxnId, XIT_k );
						if ( toCgood ) {
							if ( level_ == 2 ) {
								int votes = replyVec.size(); // how many replied
								t.setVoteInt( votes );
								// send el-'l' xit to other leaders
								t.setXit( XIT_l );
								strvec otherLeaders;
								circ.getOtherLeaders( beacon, id_, otherLeaders );
								d("a31102 %s round-1 multicast otherLeaders ..", s(id_));
								pvec(otherLeaders);
								multicast( otherLeaders, t.str(), false, replyVec );
								//OmicroClient client;
								//multicast( client, otherLeaders, t.str(), false, replyVec );
								d("a31102 %s round-1 multicast otherLeaders done", s(id_));
							} else {
								// level_ == 3  todo
							}

						} else {
							d("a3305 toCgood is false");
						}
					} else {
						d("a3306 to state A is false");
					}
				} else {
					d("a3308 i [%s] am not leader, ignore XIT_i", s(id_));
				}
				// else i am not leader, igore 'i' xit
			} else if ( xit == XIT_j ) {
				// I am follower, give my vote to leader
				sstr m(sstr("GOOD_TRXN|XIT_j|")+id_);
        		reply(m);
			} else if ( xit == XIT_l ) {
				// qwer
				sstr m(sstr("GOOD_TRXN|XIT_l|")+id_);
        		reply(m);

			    // received one XIT_l, there may be more XIT_l in next 3 seconds
				static std::unordered_map<sstr, std::vector<uint>> collectTrxn;
				static std::unordered_map<sstr, ulong> totalVotes;

				strvec otherLeaders;
				DynamicCircuit circ( nodeList_);
				bool iAmLeader = circ.getOtherLeaders( beacon, id_, otherLeaders );
				if ( iAmLeader ) {
					collectTrxn[trxnId].push_back(1);
					totalVotes[trxnId] += t.getVoteInt();

					uint twofp1 = twofplus1( otherLeaders.size() + 1);
					if ( collectTrxn[trxnId].size() >= twofp1 ) { 
						// state to D
						bool toDgood = trxnState_.goState( level_, trxnId, XIT_l );
						if ( toDgood ) {
							// todo L2 L3
							t.setXit( XIT_m );
							t.setVoteInt( totalVotes[trxnId] );
							strvec nullvec;
							d("a33221 %s round-2 multicast otherLeaders ...", s(id_));
							pvec(otherLeaders);
							multicast( otherLeaders, t.str(), false, nullvec );
							d("a33221 %s round-2 multicast otherLeaders done", s(id_));
						}
						collectTrxn.erase(trxnId);
						totalVotes.erase(trxnId);
					}
				}
			} else if ( xit == XIT_m ) {
			    // received one XIT_m, there may be more XIT_m in next 3 seconds
				// qwer
				d("a54103 %s got XIT_m", s(id_) );

				static std::unordered_map<sstr, std::vector<uint>> collectTrxn;
				static std::unordered_map<sstr, ulong> totalVotes;

				strvec otherLeaders, followers;
				DynamicCircuit circ( nodeList_);
				bool iAmLeader = circ.getOtherLeadersAndThisFollowers( beacon, id_, otherLeaders, followers );
				if ( iAmLeader ) {
					d("a52238 i am %s, a leader", s(id_) );
					collectTrxn[trxnId].push_back(1);
					totalVotes[trxnId] += t.getVoteInt();

					ulong avgVotes = totalVotes[trxnId]/otherLeaders.size();
					ulong v2fp1 = twofplus1( avgVotes );

					uint twofp1 = twofplus1( otherLeaders.size() + 1);
					d("a33039 avgVotes=%d v2fp1=%d twofp1=%d", avgVotes, v2fp1, twofp1 );

					if ( collectTrxn[trxnId].size() >= twofp1 && nodeList_.size() >= v2fp1 ) { 
						// state to E
						bool toEgood = trxnState_.goState( level_, trxnId, XIT_m );
						if ( toEgood ) {
							// todo L2 L3
							t.setXit( XIT_n );
							t.setVoteInt( avgVotes );
							strvec nullvec;
							d("a33281 %s multicast XIT_n followers ...", s(id_));
							pvec(followers);
							multicast( followers, t.str(), false, nullvec );
							d("a33281 %s multicast XIT_n followers done", s(id_));

							// i do commit too to state F
							// block_.add( t );
							d("a9999 leader commit a TRXN %s ", s(trxnId));
							trxnState_.goState( level_, trxnId, XIT_n );
							// reply back to client
							// d("a99992 conv=%ld clientconv=%ld\n", conv,  clientConv_[trxnId] );
							sstr m("GOOD_TRXN|" + id_);
							d("a4002 reply back food trxn...");
        					// kcp_server_->send_msg(clientConv_[trxnId], m);
							// qwer
						}
						collectTrxn.erase(trxnId);
						totalVotes.erase(trxnId);
					}
				} else {
					d("a52238 i am %s, NOT a leader, ignore", s(id_) );
				}
			} else if ( xit == XIT_n ) {
				// follower gets a trxn commit message
				d("a9999 follower commit a TRXN %s", s(trxnId));
			}
		}
    }
}

void *threadSendMsg(void *arg)
{
	ThreadParam *p = (ThreadParam*)arg;
	OmicroClient cli( p->srv.c_str(), p->port);
	p->reply = cli.sendMessage( p->trxn.c_str(), p->expectReply );
	return NULL;
}


bool omsession::initTrxn( OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// nodeList_ is std::vector<string>
	sstr beacon = txn.getBeacon();
	sstr trxnid = txn.getTrxnID();
	d("a80123 initTrxn() threadid=%ld beacon=[%s] trxnid=%s", pthread_self(), s(beacon), trxnid.substr(0,10).c_str() );

	// for each zone leader
	//   send leader msg: trxn, with tranit XIT_i
	// self node maybe one of the zone leaders
	DynamicCircuit circ(nodeList_);
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
	multicast( hostVec, txn.str(), true, replyVec );
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

#if 0
void omsession::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();
	if ( len < 1 ) {
		return;
	}

	d("a31303 multicast msgs to nodes %d ...", len );

	pthread_t thrd[len];
	ThreadParam thrdParam[len];
	for ( int i=0; i < len; ++i ) {
		NodeList::getData( hostVec[i], id, ip, port);
		thrdParam[i].srv = ip;
		thrdParam[i].port = atoi(port.c_str());
		thrdParam[i].trxn = trxnMsg;
		thrdParam[i].expectReply = expectReply;
		pthread_create(&thrd[i], NULL, &threadSendMsg, (void *)&thrdParam[i]);
	}

	for ( int i=0; i < len; ++i ) {
		pthread_join( thrd[i], NULL );
		if ( expectReply ) {
			if ( thrdParam[i].reply.size() > 0 ) {
				replyVec.push_back( thrdParam[i].reply );
			}
		}
	}

	d("a31303 multicast msgs to nodes %d done", len );
}
#endif

// non thread send
#if 1
void omsession::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();
	if ( len < 1 ) {
		return;
	}

	d("a31303 multicast msgs to nodes %d ...", len );

	ThreadParam thrdParam[len];
	for ( int i=0; i < len; ++i ) {
		NodeList::getData( hostVec[i], id, ip, port);
		thrdParam[i].srv = ip;
		thrdParam[i].port = atoi(port.c_str());
		thrdParam[i].trxn = trxnMsg;
		thrdParam[i].expectReply = expectReply;
		threadSendMsg((void *)&thrdParam[i]);
	}

	for ( int i=0; i < len; ++i ) {
		if ( expectReply ) {
			if ( thrdParam[i].reply.size() > 0 ) {
				replyVec.push_back( thrdParam[i].reply );
			}
		}
	}

	d("a31303 multicast msgs to nodes %d done", len );
}
#endif

/***
sstr omserver::getDataDir()
{
	sstr dir;
	dir = "../data/";
	dir += address_ + "/" + port_;
	return dir;

}
***/

void omserver::readID()
{
	sstr idpath;
	idpath = "../conf/";
	idpath += address_ + "/" + port_ + "/id.conf";

	FILE *fp = fopen(idpath.c_str(), "r");
	if ( ! fp ) {
		std::cout << "E20030 error open " << idpath << std::endl;
		exit(1);
	}

	char line[256];
	fgets(line, 250, fp);
	int len = strlen(line);
	if ( line[len-1] == '\n' ) {
		 line[len-1] = '\0';
	}

	id_ = line;
	fclose(fp);

	// check ip, port id matched in nodelist
	bool found = false;
	for (unsigned int i=0; i < nodeList_.size(); ++i ) {
		const sstr &rec = nodeList_[i];
		std::cout << "a73218 i=" << i << " node=[" << rec << "]" << std::endl;
		if ( id_ == rec ) {
			found = true;
		}
	}

	sstr id, ip, port;
	sstr fid;
	bool rc = NodeList::getData( id_, fid, ip, port);
	if  ( ! rc ) {
		std::cout << "E20331 id_ " << id_ << " not valid" << std::endl;
		exit(3);
	}

	if ( address_ != ip ) {
		std::cout << "E20032 ip " << ip << " not same as address_ " << address_ << std::endl;
		exit(4);
	}

	if ( port_ != port ) {
		std::cout << "E20033 port " << port << " not same as port_ " << port_ << std::endl;
		exit(4);
	}

	if ( ! found ) {
		std::cout << "E20031 id_ [" << id_ << "] not found in nodelist file " << std::endl;
		exit(5);
	}
}

