#include <boost/bind.hpp>
#include <signal.h>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include "essential/utility/strutil.h"
#include "omicrotrxn.h"
#include "server.hpp"
#include "dynamiccircuit.h"
#include "omutil.h"
#include "omicroclient.h"
EXTERN_LOGGING

OmicroServer::OmicroServer(const sstr& address, const sstr& port)
    : io_service_(),
    signals_(io_service_),
    stopped_(false),
    // kcp_server_(io_service_, address, port),
    test_timer_(io_service_),
	address_(address), port_(port)
{
	readID();
	i("a023381 start address_=[%s] port_=[%s]", address_.c_str(), port_.c_str() );
	kcp_server_ = new kcp_svr::server(io_service_, address_, port_);

    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(boost::bind(&OmicroServer::handle_stop, this));

    kcp_server_->set_callback(
        std::bind(&OmicroServer::event_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    hook_test_timer();

	level_ = nodeList_.getLevel();
}

OmicroServer::~OmicroServer()
{
	delete kcp_server_;
}


void OmicroServer::run()
{
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    io_service_.run();
}

void OmicroServer::handle_stop()
{
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    kcp_server_->stop();
    stopped_ = true;
}

void OmicroServer::event_callback(kcp_conv_t conv, kcp_svr::eEventType event_type, std::shared_ptr<sstr> msg)
{
	d("a2108 event_callback: conv=%ld type=%s msg=[%s]", conv, kcp_svr::eventTypeStr(event_type), (*msg).substr(0, 20).c_str());

    if (event_type == kcp_svr::eRcvMsg)
    {
        // auto send back msg for testing.
		std::shared_ptr<sstr> m1 = std::make_shared<sstr>("Server echo back: hello from server " + id_);
        kcp_server_->send_msg(conv, m1);
        //kcp_server_.send_msg(conv, msg);

		OmicroTrxn t(msg->c_str());
		bool validTrxn = t.isValidClientTrxn();
		if ( ! validTrxn ) {
			strshptr m = std::make_shared<sstr>("BAD_INVALID_TRXN|" + id_);
        	kcp_server_->send_msg(conv, m);
			return;
		}

		sstr trxnId = t.getTrxnID();
		bool isInitTrxn = t.isInitTrxn();
		bool rc;
		if ( isInitTrxn ) {
			strshptr m;
			clientConv_[trxnId] = conv;
			rc = initTrxn( conv, t );
			if ( rc ) {
				m = std::make_shared<sstr>("GOOD_TRXN|" +id_);
			} else {
				m = std::make_shared<sstr>("BAD_TRXN|" +id_);
			}
        	kcp_server_->send_msg(conv, m);
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
				strshptr m = std::make_shared<sstr>("GOOD_TRXN|XIT_j|"+id_);
        		kcp_server_->send_msg(conv, m);
			} else if ( xit == XIT_l ) {
				// qwer
				strshptr m = std::make_shared<sstr>("GOOD_TRXN|XIT_l|"+id_);
        		kcp_server_->send_msg(conv, m);

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
				auto itr = clientConv_.find(trxnId);
				if ( itr == clientConv_.end() ) {
					i("E10045 got XIT_m but cannot find clientConv_ for trxnId=%s",trxnId.substr(0,10).c_str() );
					return;
				}
				d("a55193 XIT_m clientConv=%ld", itr->second );

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
							d("a99992 conv=%ld clientconv=%ld\n", conv,  clientConv_[trxnId] );
							strshptr m = std::make_shared<sstr>(sstr("GOOD_TRXN|") + id_);
							d("a4002 reply back food trxn...");
        					kcp_server_->send_msg(clientConv_[trxnId], m);
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
	p->reply = cli.sendMessage( p->trxn.c_str(), 300);
	return NULL;
}


bool OmicroServer::initTrxn( kcp_conv_t conv, OmicroTrxn &txn )
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
void OmicroServer::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
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

void OmicroServer::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();
	if ( len < 1 ) {
		return;
	}

	d("a31303 multicast msgs to nodes %d ...", len );

	//pthread_t thrd[len];
	ThreadParam thrdParam[len];
	for ( int i=0; i < len; ++i ) {
		NodeList::getData( hostVec[i], id, ip, port);
		thrdParam[i].srv = ip;
		thrdParam[i].port = atoi(port.c_str());
		thrdParam[i].trxn = trxnMsg;
		// pthread_create(&thrd[i], NULL, &threadSendMsg, (void *)&thrdParam[i]);
		threadSendMsg((void *)&thrdParam[i]);
	}

	for ( int i=0; i < len; ++i ) {
		// pthread_join( thrd[i], NULL );
		if ( expectReply ) {
			if ( thrdParam[i].reply.size() > 0 ) {
				replyVec.push_back( thrdParam[i].reply );
			}
		}
	}

	d("a31303 multicast msgs to nodes %d done", len );
}

void OmicroServer::hook_test_timer(void)
{
    if (stopped_)
        return;

    test_timer_.expires_from_now(boost::posix_time::milliseconds(10000));
    test_timer_.async_wait(std::bind(&OmicroServer::handle_test_timer, this));
}

void OmicroServer::handle_test_timer(void)
{
    //std::cout << "."; std::cout.flush();
    hook_test_timer();

    //test_force_disconnect();
}

void OmicroServer::test_force_disconnect(void)
{
    static kcp_conv_t conv = 1000;
    kcp_server_->force_disconnect(conv);
    conv++;
}

sstr OmicroServer::getDataDir()
{
	sstr dir;
	dir = "../data/";
	dir += address_ + "/" + port_;
	return dir;

}

void OmicroServer::readID()
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

