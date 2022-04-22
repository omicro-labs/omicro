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
#include <boost/bind/bind.hpp>
#include <signal.h>
#include <cstdlib>
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
#include "omsync.h"
#include "omsession.h"
#include "omstrsplit.h"
#include "omicrokey.h"

EXTERN_LOGGING
using namespace boost::asio::ip;
using bcode = boost::system::error_code;


omserver::omserver( boost::asio::io_context &io_context, const sstr &srvip, const sstr &port)
      : io_context_(io_context), 
	    acceptor_(io_context, tcp::endpoint(boost::asio::ip::address::from_string(srvip.c_str()), atoi(port.c_str()) ))
{
    address_ = srvip;
    port_ = port;

    readID();
    readPubkey();
    readSeckey();
    readSrvportPubkey();

    level_ = nodeList_.getLevel();
	srvport_ = address_ + ":" + port_;

	sstr dataDir = getDataDir(); 
	i("Data dir is [%s]", dataDir.c_str());
	blockMgr_.setSrvPort( address_, port_);
	blockMgr_.setDataDir( dataDir );

	cleanupTimer_ = new btimer( io_context_ );

	cleanupTimer_->expires_at(cleanupTimer_->expiry() + boost::asio::chrono::seconds(300));
	cleanupTimer_->async_wait(boost::bind(&omserver::doCleanup, this ));

    do_accept();
	i("omserver is ready");

}

omserver::~omserver()
{
	delete cleanupTimer_;
}

void omserver::do_accept()
{
    acceptor_.async_accept(
          [this](bcode ec, tcp::socket accpt_sock) {
            if (!ec)
            {
				//printf("server accepted connection newsession ...\n");
                std::shared_ptr<omsession> sess = std::make_shared<omsession>(io_context_, *this, std::move(accpt_sock));
                sess->start();
				//printf("server connection newsession is done\n");
            }

            do_accept();
    });

}

sstr omserver::getDataDir() const
{
	::mkdir( "../data/", 0700 );
	sstr dip = sstr("../data/") + address_;
	::mkdir( dip.c_str(), 0700 );

	sstr dir = dip + "/" + port_;
	return dir;
}

void omserver::readPubkey()
{
	sstr idpath;
	idpath = "../conf/";
	idpath += address_ + "/" + port_ + "/publickey";

	FILE *fp = fopen(idpath.c_str(), "r");
	if ( ! fp ) {
		i("E21030 error open [%s]", s(idpath) );
		exit(1);
	}

	char line[2048];
	fgets(line, 2048, fp);
	int len = strlen(line);
	if ( line[len-1] == '\n' ) {
		 line[len-1] = '\0';
	}

	pubKey_ = line;
	fclose(fp);
}

void omserver::readSeckey()
{
	sstr idpath;
	idpath = "../conf/";
	idpath += address_ + "/" + port_ + "/secretkey";

	FILE *fp = fopen(idpath.c_str(), "r");
	if ( ! fp ) {
		i("E21034 error open [%s]", s(idpath) );
		exit(1);
	}

	char line[3048];
	fgets(line, 3048, fp);
	int len = strlen(line);
	if ( line[len-1] == '\n' ) {
		 line[len-1] = '\0';
	}

	secKey_ = line;
	fclose(fp);
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

void omserver::readSrvportPubkey()
{
	sstr idpath;
	idpath = "../conf/";
	idpath += address_ + "/" + port_ + "/srvport_pubkey";

	FILE *fp = fopen(idpath.c_str(), "r");
	if ( ! fp ) {
		i("E21030 error open [%s]", s(idpath) );
		exit(1);
	}

	char line[1400];

	int len;
	while ( NULL != fgets(line, 1400, fp) ) {
		len = strlen(line);
		if ( line[len-1] == '\n' ) {
			 line[len-1] = '\0';
		}
		OmStrSplit sp(line, '|');
		srvport_pubkey_.emplace(sp[0], sp[1]);
	}
	fclose(fp);
}

void omserver::getPubkey( const sstr &srvport, sstr &pubkey )
{
	pubkey = srvport_pubkey_[srvport];
}

void omserver::onRecvK( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t )
{
	strvec otherLeaders;
	strvec followers;
	DynamicCircuit circ( nodeList_);

	bool iAmLeader = circ.getOtherLeadersAndThisFollowers( beacon, id_, otherLeaders, followers );
	if ( ! iAmLeader ) {
		d("a22034 %s got XIT_k, i am not leader", s(id_) );
		return;
	}
	d("a30024 onRecvK() i am leader client=[%s] sid=[%s] ...", s(clientIP), s(sid));

	Byte curState;
	trxnState_.getState( trxnId, curState );
	if ( curState >= ST_C ) {
		d("a02824 doRecvK from=%s state=%c already at or passed ST_C, ignore",  s(t.sender_), curState );
		return;
	}

	collectKTrxn_.add(trxnId, 1);

	d("a20733 from=%s got XIT_k am leader increment collectTrxn[trxnId]", s(t.sender_));
		
	uint twofp1 = twofplus1( followers.size() + 1);
	d("a53098 doRecvK from=%s twofp1=%d followers=%d", s(t.sender_), twofp1, followers.size() );
	uint recvcnt = collectKTrxn_.get(trxnId);
	if ( recvcnt >= twofp1 ) { 
		d("a2234 from=%s %s got enough quorum XIT_k, good, leader,  rcvcnt=%d >= twofp1=%d trnxId=%s", s(t.sender_), s(id_), recvcnt, twofp1, s(trxnId) );
		// state to C
		trxnState_.setState( trxnId, ST_C );
		d("a331808 from=%s XIT_k to toCgood true. mcast XIT_l to other leaders", s(t.sender_));
		// todo L2 L3
		t.setXit( XIT_l );
		strvec nullvec;
		t.srvport_ = srvport_; 
		sstr alldat; t.allstr(alldat);
		multicast( OM_TXN, otherLeaders, alldat, false, nullvec ); // XIT_l
		d("a39221 from=%s %s multicast otherLeaders done t.srvport_=[%s]", s(t.sender_), s(id_), s(t.srvport_) );
		collectKTrxn_.erase(trxnId);
	} else {
		d("a21842 from=%s %s got XIT_k but notenough quorum, a leader, rcvcnt=%d < twofp1=%d", s(t.sender_), s(id_), recvcnt, twofp1 );
	}
}

void omserver::onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t )
{
	strvec otherLeaders;
	DynamicCircuit circ( nodeList_);
	bool iAmLeader = circ.getOtherLeaders( beacon, id_, otherLeaders );
	if ( ! iAmLeader ) {
		d("a22034 %s got XIT_l, i am not leader", s(id_) );
		return;
	}
	d("a30024 onRecvL() i am leader client=[%s] sid=[%s] tryRecvL ...", s(clientIP), s(sid));

	Byte curState;
	trxnState_.getState( trxnId, curState );
	if ( curState >= ST_D ) {
		d("a00821 doRecvL from=%s state=%c already at or passed ST_D, ignore",  s(t.sender_), curState );
		return;
	}

	d("a5549381 from=%s onRecvL check that server is in ST_C, go ahead", s(t.sender_) );

	collectLTrxn_.add(trxnId, 1);

	d("a3733 from=%s got XIT_l am leader increment collectTrxn[trxnId]", s(t.sender_));
		
	uint twofp1 = twofplus1( otherLeaders.size() + 1);
	d("a53098 doRecvL from=%s twofp1=%d otherleaders=%d", s(t.sender_), twofp1, otherLeaders.size() );
	uint recvcnt = collectLTrxn_.get(trxnId);
	if ( recvcnt >= twofp1 ) { 
		d("a2234 from=%s %s got enough quorum XIT_l, good, leader,  rcvcnt=%d >= twofp1=%d trnxId=%s", s(t.sender_), s(id_), recvcnt, twofp1, s(trxnId) );
		// state to D
		trxnState_.setState( trxnId, ST_D );
		d("a331208 from=%s XIT_l to toDgood true", s(t.sender_));
		// todo L2 L3
		t.setXit( XIT_m );
		strvec nullvec;
		//pvec(otherLeaders);
		t.srvport_ = srvport_; 
		sstr alldat; t.allstr(alldat);
		//d("a33221 %s round-2 multicast otherLeaders trnmsg=[%s] ...", s(id_), s(dat) );
		multicast( OM_TXN, otherLeaders, alldat, false, nullvec );
		d("a33221 from=%s %s round-2 multicast otherLeaders done t.srvport_=[%s]", s(t.sender_), s(id_), s(t.srvport_) );

		collectLTrxn_.erase(trxnId);
	} else {
		d("a22342 from=%s %s got XIT_l but notenough quorum, a leader, rcvcnt=%d < twofp1=%d", s(t.sender_), s(id_), recvcnt, twofp1 );
	}
}

// L2
void omserver::onRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t )
{
    strvec otherLeaders, followers;
    DynamicCircuit circ( nodeList_);
    bool iAmLeader = circ.getOtherLeadersAndThisFollowers( beacon, id_, otherLeaders, followers );
    if ( ! iAmLeader ) {
    	d("a52238 i am %s, NOT a leader, ignore", s(id_) );
		return;
	}

	d("a399302 onRecvM otherLeaders.size=%d", otherLeaders.size() );

	Byte curState;
	trxnState_.getState( trxnId, curState );
	if ( curState >= ST_E ) {
		d("a00821 doRecvL from=%s state=%c already at or passed ST_D, ignore", s(t.sender_), curState);
		return;
	}

	collectMTrxn_.add(trxnId, 1);
	ulong trxnVotes = collectMTrxn_.get(trxnId);
	uint twofp1 = twofplus1( otherLeaders.size() + 1);
	d("a33039 doRecvM  from=%s twofp1=%d trxnVotes=%d", s(t.sender_), twofp1, trxnVotes );

	if ( trxnVotes >= twofp1 ) {
		// state to E
		d("a2235 from=%s %s got enough quorum XIT_m, good, leader, trxnid=%s", s(t.sender_), s(id_), s(trxnId) );
		trxnState_.setState( trxnId, ST_E );
		if ( 1 ) {
			d("a02227 from=%s XIT_m toEgood ST_E true", s(t.sender_));
			// todo L2 L3
			t.setXit( XIT_n );
			strvec nullvec;
			d("a33281 from=%s %s multicast XIT_n followers ...", s(t.sender_), s(id_));

			//i("a99390 todo followers:");
			//pvectag("tagfollower:srv", followers);

			t.srvport_ = srvport_;
			sstr alldat; t.allstr(alldat);
			omserver::multicast( OM_TXN, followers, alldat, false, nullvec );
			d("a33281 from=%s %s multicast XIT_n followers done", s(t.sender_), s(id_));

			trxnState_.setState( trxnId, ST_F );
			blockMgr_.receiveTrxn( t );
			d("a99 leader from=[%s] commit TRXN %s peer:[%s]", s(t.sender_), s(trxnId), s(t.srvport_) );
		} 
		collectMTrxn_.erase(trxnId);
	} else {
		d("a72228 XIT_m from=%s notenough quorum, XIT_n not sent ", s(t.sender_));
	}

}

int omserver::multicast( char msgType, const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
{
	sstr id, ip, port;
	int len = hostVec.size();
	if ( len < 1 ) {
		i("E50292 multicast hostVec.size < 1");
		return false;
	}

	d("a31303 multicast send msgs to nodes %d expectReply=%d ...", len, expectReply );
	replyVec.clear();

	pthread_t thrd[len];
	ThreadParam thrdParam[len];
	sstr srvport, pubkey, dat;
	bool useThreads = true;
	int trc;

	for ( int j=0; j < len; ++j ) {
		NodeList::getData( hostVec[j], id, ip, port);
		thrdParam[j].srv = ip;
		thrdParam[j].port = atoi(port.c_str());
		thrdParam[j].msgType = msgType;
		srvport = ip + ":" + port;

		getPubkey( srvport, pubkey );
		OmicroTrxn t( trxnMsg.c_str() );
		t.makeNodeSignature(pubkey);

		d("a10827 from=%s in multicast get target srvport pubkey of target: %s:%s", s(t.sender_), s(ip), s(port) );
		//d("a10827 pubkey [%s]", s(pubkey) );
		d("a30062 from=%s in multicast my t.srvport_=[%s]", s(t.sender_), s(t.srvport_) );

		#ifdef OM_DEBUG
		sstr trxndata; t.getTrxnData(trxndata);
		bool rc1 = OmicroNodeKey::verifySB3( trxndata, t.signature_, t.cipher_, secKey_);
		d("a2999 in multicast after t.makeNodeSignature verifySB3 rc1=%d", rc1 );
		bool rc = t.validateTrxn( secKey_ );
		d("a2999 in multicast after t.makeNodeSignature t.validateTrxn rc=%d", rc );
		#endif

		t.allstr(thrdParam[j].trxn);
		thrdParam[j].expectReply = expectReply;
		if ( useThreads ) {
			trc = pthread_create(&thrd[j], NULL, &threadSendMsg, (void *)&thrdParam[j]);
			if ( 0 == trc ) {
				d("a43308 create thread OK trc=%d", trc );
			} else {
				i("E43308 failed to create thread trc=%d", trc );
			}
		} else {
			threadSendMsg( (void *)&thrdParam[j] );
		}
	}

	int connected = 0;
	for ( int j=0; j < len; ++j ) {
		if ( useThreads ) {
			pthread_join( thrd[j], NULL );
		}

		if ( expectReply ) {
			d("a59031 pthread_join i=%d done expectReply=1", i );
			if ( thrdParam[j].reply.size() > 0 ) {
				replyVec.push_back( thrdParam[j].reply );
			}
		}

		if ( thrdParam[j].reply != "NOCONN" ) {
			++connected;
		}
	}

	d("a31303 multicast msgs to nodes %d done expectReply=%d replied=%d connected=%d", len, expectReply, replyVec.size(), connected );
	return connected;
}

void *threadSendMsg(void *arg)
{
    ThreadParam *p = (ThreadParam*)arg;
    OmicroClient cli( p->srv.c_str(), p->port);
	if ( ! cli.connectOK() ) {
		p->reply = "NOCONN";
		return NULL;
	}

	d("a30114 cli.sendMessage ... " );
    p->reply = cli.sendMessage( p->msgType,  p->trxn.c_str(), p->expectReply );
	d("a30114 cli.sendMessage returned p->expectReply=%d p->reply=[%s]", p->expectReply, s(p->reply) );
    return NULL;
}

void omserver::onRecvQueryK( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t )
{
	strvec followers;
	DynamicCircuit circ( nodeList_);

	bool iAmLeader = circ.isLeader( beacon, id_, false, followers );
	if ( ! iAmLeader ) {
		d("a25034 %s got query XIT_k, i am not leader, ignore", s(id_) );
		return;
	}
	d("a30024 onRecvQueryK() i am leader client=[%s] sid=[%s] ...", s(clientIP), s(sid));

	Byte curState;
	trxnState_.getState( trxnId, curState );
	if ( curState >= ST_C ) {
		d("a02824 doRecvK from=%s state=%c already at or passed ST_C, ignore",  s(t.sender_), curState );
		return;
	}

	collectQueryKTrxn_.add(trxnId, 1);
		
	uint twofp1 = twofplus1( followers.size() + 1);
	d("a53098 doRecvK from=%s twofp1=%d followers=%d", s(t.sender_), twofp1, followers.size() );
	uint recvcnt = collectKTrxn_.get(trxnId);
	if ( recvcnt >= twofp1 ) { 
		d("a2234 from=%s %s got enough quorum query XIT_k, good, leader, cnt=%d >= twofp1=%d trnxId=%s", s(t.sender_), s(id_), recvcnt, twofp1, s(trxnId) );
		// state to C
		trxnState_.setState( trxnId, ST_C );
		collectQueryKTrxn_.erase(trxnId);
	} else {
		d("a27842 from=%s %s got query XIT_k but notenough quorum, a leader, rcvcnt=%d < twofp1=%d", s(t.sender_), s(id_), recvcnt, twofp1 );
	}
}

void omserver::doCleanup()
{
	// cleanup votes and collections of trxnID
	collectKTrxn_.cleanup( 240 );
	collectQueryKTrxn_.cleanup( 240 );
    collectLTrxn_.cleanup( 240 );
	collectMTrxn_.cleanup( 240 );
	qResult_.cleanup( 240 );

	cleanupTimer_->expires_at(cleanupTimer_->expiry() + boost::asio::chrono::seconds(300));
	cleanupTimer_->async_wait(boost::bind(&omserver::doCleanup, this ));
}
