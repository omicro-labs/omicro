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

OmicroServer::OmicroServer(const sstr& address, const sstr& port)
    : io_service_(),
    signals_(io_service_),
    stopped_(false),
    // kcp_server_(io_service_, address, port),
    test_timer_(io_service_),
	address_(address), port_(port)
{
	readID();
	std::cout << "a023381 start address_=[" << address_ << "] port_=[" << port_ << "]" << std::endl;
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
    std::cout << "event_callback: conv=" << conv << " type:" << kcp_svr::eventTypeStr(event_type) << " msg: " << *msg << std::endl;
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
						t.setXit( XIT_j );
						strvec replyVec;
						multicast( followers, t.str(), true, replyVec );

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
								multicast( otherLeaders, t.str(), false, replyVec );
							} else {
								// level_ == 3  todo
							}

						} else {
							std::cout << "a3305 toCgood is false" << std::endl;
						}
					} else {
						std::cout << "a3306 to state A is false" << std::endl;
					}
				}  else {
					std::cout << "a3308 i am not leader, ignore XIT_i" << std::endl;
				}
				// else i am not leader, igore 'i' xit
			} else if ( xit == XIT_j ) {
				// I am follower, give my vote to leader
				strshptr m = std::make_shared<sstr>("GOOD_TRXN|"+id_);
        		kcp_server_->send_msg(conv, m);
			} else if ( xit == XIT_l ) {
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
							multicast( otherLeaders, t.str(), false, nullvec );
						}
						collectTrxn.erase(trxnId);
						totalVotes.erase(trxnId);
					}
				}
			} else if ( xit == XIT_m ) {
			    // received one XIT_m, there may be more XIT_m in next 3 seconds
				// qwer
				auto itr = clientConv_.find(trxnId);
				if ( itr == clientConv_.end() ) {
					std::cout << "E10045 got XIT_m but cannot find clientConv_ for trxnId=" << trxnId << std::endl;
					return;
				}

				static std::unordered_map<sstr, std::vector<uint>> collectTrxn;
				static std::unordered_map<sstr, ulong> totalVotes;

				strvec otherLeaders, followers;
				DynamicCircuit circ( nodeList_);
				bool iAmLeader = circ.getOtherLeadersAndFollowers( beacon, id_, otherLeaders, followers );
				if ( iAmLeader ) {
					collectTrxn[trxnId].push_back(1);
					totalVotes[trxnId] += t.getVoteInt();

					ulong avgVotes = totalVotes[trxnId]/otherLeaders.size();
					ulong v2fp1 = twofplus1( avgVotes );

					uint twofp1 = twofplus1( otherLeaders.size() + 1);
					if ( collectTrxn[trxnId].size() >= twofp1 && nodeList_.size() >= v2fp1 ) { 
						// state to E
						bool toEgood = trxnState_.goState( level_, trxnId, XIT_m );
						if ( toEgood ) {
							// todo L2 L3
							t.setXit( XIT_n );
							t.setVoteInt( avgVotes );
							strvec nullvec;
							multicast( followers, t.str(), false, nullvec );

							// i do commit too to state F
							// block_.add( t );
							std::cout <<"a9999 leader commit a TRXN " << trxnId << std::endl;
							trxnState_.goState( level_, trxnId, XIT_n );
							// reply back to client
							std::cout <<"a99992 conv=" << conv << "  clientconv=" << clientConv_[trxnId] << std::endl;
							strshptr m = std::make_shared<sstr>(sstr("GOOD_TRXN|") + id_);
        					kcp_server_->send_msg(clientConv_[trxnId], m);
						}
						collectTrxn.erase(trxnId);
						totalVotes.erase(trxnId);
					}
				}
			} else if ( xit == XIT_n ) {
				// follower gets a trxn commit message
				std::cout <<"a9999 follower commit a TRXN " << trxnId << std::endl;
				
			}
		}
    }
}

void *threadSendMsg(void *arg)
{
	ThreadParam *p = (ThreadParam*)arg;
	OmicroClient cli( p->srv.c_str(), p->port, 10);
	p->reply = cli.sendMessage( p->trxn.c_str(), 100);
	return NULL;
}


bool OmicroServer::initTrxn( kcp_conv_t conv, OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// nodeList_ is std::vector<string>
	sstr beacon = txn.getBeacon();
	sstr trxnid = txn.getTrxnID();
	std::cout << "a80123 OmicroServer::initTrxn() threadid=" << pthread_self() << std::endl;
	std::cout << "a80124 beacon=[" << beacon << "]" << std::endl;
	std::cout << "a80124 trxnid=" << trxnid << std::endl;

	// for each zone leader
	//   send leader msg: trxn, with tranit XIT_i
	// self node maybe one of the zone leaders
	DynamicCircuit circ(nodeList_);
	strvec hostVec;
	circ.getZoneLeaders( beacon, hostVec );

	txn.setNotInitTrxn();
	txn.setXit( XIT_i );

	strvec replyVec;
	multicast( hostVec, txn.str(), true, replyVec );

	uint numReply = replyVec.size();
	uint onefp1 = onefplus1(hostVec.size());
	std::cout << "a4550 numReply=" << numReply << " onefp1=" << onefp1 << std::endl;
	if ( numReply >= onefp1 ) {
		return true;
	} else {
		return false;
	}
}

void OmicroServer::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();
	if ( len < 1 ) {
		return;
	}

	std::cout << "a31303 multicast msgs to nodes " << len << std::endl;

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

	std::cout << "a31304 multicast msgs done" << std::endl;
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

