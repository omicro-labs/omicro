#include <boost/bind.hpp>
#include <signal.h>
#include <cstdlib>
#include <pthread.h>
#include <malloc.h>

#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <mutex>

#include "omicrotrxn.h"
#include "server.hpp"
#include "dynamiccircuit.h"
#include "omutil.h"
#include "omicroclient.h"
#include "ommsghdr.h"
#include "omsync.h"
#include "omsession.h"

EXTERN_LOGGING
using namespace boost::asio::ip;

omserver::omserver( boost::asio::io_context &io_context, const sstr &srvip, const sstr &port)
      : io_context_(io_context),
	    acceptor_(io_context, tcp::endpoint(boost::asio::ip::address::from_string(srvip.c_str()), atoi(port.c_str()) ))
{
    address_ = srvip;
    port_ = port;
    readID();
    level_ = nodeList_.getLevel();

    do_accept();
}

void omserver::do_accept()
{
    acceptor_.async_accept(
          [this](bcode ec, tcp::socket acc_sock)
          {
            if (!ec)
            {
                std::shared_ptr<omsession> sess = std::make_shared<omsession>(io_context_, *this, std::move(acc_sock));
                sess->start();
            }

            do_accept();
    });

}

sstr omserver::getDataDir() const
{
	sstr dir;
	dir = "../data/";
	dir += address_ + "/" + port_;
	return dir;
}

void omserver::readID()
{
	sstr idpath;
	idpath = "../conf/";
	idpath += address_ + "/" + port_ + "/id.conf";

	FILE *fp = fopen(idpath.c_str(), "r");
	if ( ! fp ) {
		i("E20030 error open [%s]", s(idpath) );
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
		d("a73218 i=%d node=p%s]", i, s(rec) );
		if ( id_ == rec ) {
			found = true;
		}
	}

	sstr id, ip, port;
	sstr fid;
	bool rc = NodeList::getData( id_, fid, ip, port);
	if  ( ! rc ) {
		i("E20331 id_=[%s] not valid", s(id_) );
		exit(3);
	}

	if ( address_ != ip ) {
		i("E20032 ip=%s different from address_=%s", s(ip), s(address_) );
		exit(4);
	}

	if ( port_ != port ) {
		i("E20033 port=%s different from port_=%s", s(port), s(port_) );
		exit(4);
	}

	if ( ! found ) {
		i("E20031 id_=[%s] not found in nodelist file", s(id_) );
		exit(5);
	}
}

// L2
void omserver::onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t )
{
	strvec otherLeaders;
	DynamicCircuit circ( nodeList_);
	bool iAmLeader = circ.getOtherLeaders( beacon, id_, otherLeaders );
	if ( ! iAmLeader ) {
		d("a2234 %s got XIT_l, i am not leader", s(id_) );
		return;
	}

	d("a30024 i am leader client=[%s]", s(clientIP));
	// make sure state C is done first
   	//auto self(shared_from_this());
	boost::asio::post(io_context_, [&]() {
		bool &passedC = passedC_[trxnId];
		{
			d("a111023 unique_lock client=[%s] ...",  s(clientIP));
			std::unique_lock<std::mutex> lk(getMutx(trxnId));
			d("a111024 cv_wait_timeout client=[%s] passedC_=%d sid=[%s] ...", s(clientIP), passedC, s(sid));
			int trc = cv_wait_timeout( passedC, lk, getCond(trxnId), 5);
			if ( trc < 0 ) {
				d("a72203 cv_wait_timeout got timeout, skip rest client=[%s]", s(clientIP) );
				passedC_.erase(trxnId);
				return;
			}
			d("a555039 cv_wait_timeout NO timeout passedC_=%d sid=[%s]", passedC, s(sid) );
			passedC_.erase(trxnId);
		}

		collectTrxn_[trxnId].push_back(1);
		totalVotes_[trxnId] += t.getVoteInt();
		d("a3733 got XIT_l am leader increment collectTrxn[trxnId]");

		uint twofp1 = twofplus1( otherLeaders.size() + 1);
		d("a53098 twofp1=%d otherleaders=%d", twofp1, otherLeaders.size() );
		uint recvcnt = collectTrxn_[trxnId].size();
		if ( recvcnt >= twofp1 ) { 
			// state to D
			bool toDgood = trxnState_.goState( level_, trxnId, XIT_l );
			if ( toDgood ) {
				d("a331208 from XIT_l to toDgood true");
				// todo L2 L3
				t.setXit( XIT_m );
				t.setVoteInt( totalVotes_[trxnId] );
				strvec nullvec;
				d("a33221 %s round-2 multicast otherLeaders ...", s(id_));
				pvec(otherLeaders);
				multicast( otherLeaders, t.str(), false, nullvec );
				d("a33221 %s round-2 multicast otherLeaders done", s(id_));
			} else {
				d("a4457 %s XIT_l toDgood is false", s(id_));
			}
			collectTrxn_.erase(trxnId);
			totalVotes_.erase(trxnId);
		} else {
			d("a2234 %s got XIT_l, a leader, but rcvcnt=%d < twofp1=%d, nostart round-2", s(id_), recvcnt, twofp1 );
		}
	} );
}

// L2
void omserver::onRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t )
{
    strvec otherLeaders, followers;
    DynamicCircuit circ( nodeList_);
    bool iAmLeader = circ.getOtherLeadersAndThisFollowers( beacon, id_, otherLeaders, followers );
    if ( ! iAmLeader ) {
    	d("a52238 i am %s, NOT a leader, ignore", s(id_) );
		return;
	}

	d("a52238 i am %s, a leader", s(id_) );
	collectTrxn_[trxnId].push_back(1);
	totalVotes_[trxnId] += t.getVoteInt();

	ulong avgVotes = totalVotes_[trxnId]/otherLeaders.size();
	ulong v2fp1 = twofplus1( avgVotes );

	uint twofp1 = twofplus1( otherLeaders.size() + 1);
	d("a33039 avgVotes=%d v2fp1=%d twofp1=%d", avgVotes, v2fp1, twofp1 );

	if ( collectTrxn_[trxnId].size() >= twofp1 && nodeList_.size() >= v2fp1 ) { 
		// state to E
		bool toEgood = trxnState_.goState( level_, trxnId, XIT_m );
		if ( toEgood ) {
			d("a02227 from XIT_m toEgood true");
			// todo L2 L3
			t.setXit( XIT_n );
			t.setVoteInt( avgVotes );
			strvec nullvec;
			d("a33281 %s multicast XIT_n followers ...", s(id_));
			pvec(followers);
			omserver::multicast( followers, t.str(), false, nullvec );
			d("a33281 %s multicast XIT_n followers done", s(id_));

			// i do commit too to state F
			// block_.add( t );
			d("a9999 leader commit a TRXN %s ", s(trxnId));
			trxnState_.goState( level_, trxnId, XIT_n );
			// reply back to client
			// d("a99992 conv=%ld clientconv=%ld\n", conv,  clientConv_[trxnId] );
			sstr m( sstr("GOOD_TRXN|") + id_);
			d("a4002 reply back food trxn...");
			// kcp_server_->send_msg(clientConv_[trxnId], m);
			// qwer
		} else {
			d("a72128 from XIT_m toEgood false");
		}
		collectTrxn_.erase(trxnId);
		totalVotes_.erase(trxnId);
	}
}


std::mutex& omserver::getMutx(const sstr &trxnId)
{
	auto itr = stmtx_.find(trxnId);
	if ( itr == stmtx_.end() ) {
		stmtx_.emplace(std::piecewise_construct, std::forward_as_tuple(trxnId), std::forward_as_tuple());
		return stmtx_[trxnId];
	} else {
		return itr->second;
	}
}

std::condition_variable& omserver::getCond( const sstr &trxnId)
{
	auto itr = stcv_.find(trxnId);
	if ( itr == stcv_.end() ) {
		stcv_.emplace(std::piecewise_construct, std::forward_as_tuple(trxnId), std::forward_as_tuple());
		return stcv_[trxnId];
	} else {
		return itr->second;
	}
}

void omserver::clearMutxCond( const sstr &trxnId )
{
	stmtx_.erase( trxnId );
	stcv_.erase( trxnId );
}



#if 0
void omserver::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
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
void omserver::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();
	if ( len < 1 ) {
		d("a50221 multicast hostVec is empty, noop");
		return;
	}

	replyVec.clear();

	d("a31303 multicast msgs to nodes %d expectReply=%d ...", len, expectReply );

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

	d("a31303 multicast msgs to nodes %d done expectReply=%d replied=%d", len, expectReply, replyVec.size() );
}

void *threadSendMsg(void *arg)
{
    ThreadParam *p = (ThreadParam*)arg;
    OmicroClient cli( p->srv.c_str(), p->port);
    p->reply = cli.sendMessage( p->trxn.c_str(), p->expectReply );
    return NULL;
}

