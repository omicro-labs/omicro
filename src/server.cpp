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
    kcp_server_(io_service_, address, port),
    test_timer_(io_service_),
	address_(address), port_(port)
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(boost::bind(&OmicroServer::handle_stop, this));

    kcp_server_.set_callback(
        std::bind(&OmicroServer::event_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    hook_test_timer();

	level_ = nodeList_.getLevel();
	readID();
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
    kcp_server_.stop();
    stopped_ = true;
}

void OmicroServer::event_callback(kcp_conv_t conv, kcp_svr::eEventType event_type, std::shared_ptr<sstr> msg)
{
    std::cout << "event_callback:" << conv << " type:" << kcp_svr::eventTypeStr(event_type) << " msg: " << *msg << std::endl;
    if (event_type == kcp_svr::eRcvMsg)
    {
        // auto send back msg for testing.
		std::shared_ptr<sstr> m1 = std::make_shared<sstr>("Server echo back: hello from server");
        kcp_server_.send_msg(conv, m1);
        //kcp_server_.send_msg(conv, msg);

		OmicroTrxn t(msg->c_str());
		bool validTrxn = t.isValidClientTrxn();
		if ( ! validTrxn ) {
			strshptr m = std::make_shared<sstr>("BAD_INVALID_TRXN");
        	kcp_server_.send_msg(conv, m);
			return;
		}

		bool isInitTrxn = t.isInitTrxn();
		bool rc;
		if ( isInitTrxn ) {
			strshptr m;
			rc = initTrxn( conv, t );
			if ( rc ) {
				m = std::make_shared<sstr>("GOOD_TRXN");
			} else {
				m = std::make_shared<sstr>("BAD_TRXN");
			}
        	kcp_server_.send_msg(conv, m);
		} else {
			Byte xit = t.getXit();
			if ( xit == XIT_i ) {
				// I got 'i', I must be a leader
				sstr beacon = t.getBeacon();
				DynamicCircuit circ( nodeList_);
				strvec followers;
				bool iAmLeader = circ.isLeader( beacon, id_, followers );
				if ( iAmLeader ) {
					// state to A
					sstr trxnId = t.getTrxnID();
					bool goodXit = trxnState_.goState( level_, trxnId, XIT_i );
					if ( goodXit ) {
						// send XIT_j to all followers in this leader zone
						t.setXit( XIT_j );
						strvec replyVec;
						multicast( followers, t.str(), replyVec );

						// got replies from followers, state to C
						bool toCgood = trxnState_.goState( level_, trxnId, XIT_k );
						if ( toCgood ) {
							if ( level_ == 2 ) {
								// send el-'l' xit to other leaders
								t.setXit( XIT_l );
								t.setVoteInt( replyVec.size() );
								// qwer
								strvec otherLeaders;
								circ.getOtherLeaders( beacon, id_, otherLeaders );

							} else {
								// level_ == 3  todo
							}


						}
					}
				} 
				// else i am not leader, igore 'i' xit
			} else if ( xit == XIT_j ) {
				// I am follower, give my vote to leader
				bool validTrxn = t.isValidClientTrxn();
				if ( validTrxn ) {
					strshptr m = std::make_shared<sstr>("GOOD_TRXN");
        			kcp_server_.send_msg(conv, m);
				}
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
	std::cout << "a80124 beacon=" << beacon << std::endl;
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
	multicast( hostVec, txn.str(), replyVec );
	int numReply = replyVec.size();
	if ( numReply >= onefplus1(hostVec.size()) ) {
		return true;
	} else {
		return false;
	}
}

void OmicroServer::multicast( const strvec &hostVec, const sstr &trxnMsg, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();

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
		if ( thrdParam[i].reply.size() > 0 ) {
			replyVec.push_back( thrdParam[i].reply );
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
    kcp_server_.force_disconnect(conv);
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
	fgets(line, 200, fp);
	int len = strlen(line);
	if ( line[len-1] == '\n' ) {
		 line[len-1] = '\0';
	}

	id_ = line;
	fclose(fp);

	// check ip, port id matched in nodelist
	sstr id, ip, port;
	bool found = false;
	for (unsigned int i=0; i < nodeList_.size(); ++i ) {
		const sstr &rec = nodeList_[i];
		if ( id == rec ) {
			found = true;
		}
	}

	if ( ! found ) {
		std::cout << "E20031 id_ " << id_ << " not found in nodelist file " << std::endl;
		exit(2);
	}
}

