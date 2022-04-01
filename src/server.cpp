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

EXTERN_LOGGING
using namespace boost::asio::ip;

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
	blockMgr_.setDataDir( dataDir );

	timer1_ = new btimer( io_context_ );
	timer2_ = new btimer( io_context_ );
	waitCCount_ = 0;
	waitDCount_ = 0;

    do_accept();
	i("omserver is ready");

}

omserver::~omserver()
{
	delete timer1_;
	delete timer2_;
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

void omserver::onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t )
{
	strvec otherLeaders;
	DynamicCircuit circ( nodeList_);
	bool iAmLeader = circ.getOtherLeaders( beacon, id_, otherLeaders );
	if ( ! iAmLeader ) {
		d("a22034 %s got XIT_l, i am not leader", s(id_) );
		return;
	}
	d("a30024 onRecvL() i am leader client=[%s] sid=[%s] tryRecvL ...", s(clientIP), s(sid));

	tryRecvL( beacon, trxnId, clientIP, sid, otherLeaders, t);
}

void omserver::tryRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
					    const strvec &otherLeaders,  OmicroTrxn &t )
{
	Byte curState = ST_0;
	bool rc = trxnState_.getState(trxnId, curState);
	if ( rc && curState > ST_C ) {
		// later comers
		d("a70012 tryRecvL T_l curState=%c passed ST_C already, late msg, ignored", curState);
		return;
	}

	if ( rc && curState == ST_C ) {
		d("a81116 curState == ST_C doRecvL ...");
		doRecvL( beacon, trxnId, clientIP, sid, otherLeaders, t );
	} else {
		// debug only
		++waitCCount_;
		if ( waitCCount_ > 100 ) {
			d("a9207 error waitCCount_ > 100 give up wait");
			waitCCount_ = 0;
			return;
		}

		d("a81116 curState != ST_C (curstte=%c) wait 100 millisecs then tryRecvL ...", curState );
		d("a81116 curState != ST_C sid=[%s] trxnid=[%s]", s(sid), trxnId.substr(0,50).c_str() );
		timer1_->expires_at(timer1_->expiry() + boost::asio::chrono::milliseconds(100));
		timer1_->async_wait(boost::bind(&omserver::tryRecvL, this, beacon, trxnId, clientIP, sid, otherLeaders, boost::ref(t) ));
	}
	//doRecvL( beacon, trxnId, clientIP, sid, otherLeaders, t );
}

void omserver::doRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
					    const strvec &otherLeaders,  OmicroTrxn &t )
{


	d("a5549381 onRecvL check that server is in ST_C, go ahead");
	collectTrxn_[trxnId].push_back(1);
	totalVotes_[trxnId] += t.getVoteInt();
	d("a3733 got XIT_l am leader increment collectTrxn[trxnId]");
		
	uint twofp1 = twofplus1( otherLeaders.size() + 1);
	d("a53098 twofp1=%d otherleaders=%d", twofp1, otherLeaders.size() );
	uint recvcnt = collectTrxn_[trxnId].size();
	if ( recvcnt >= twofp1 ) { 
		d("a2234 %s got XIT_l, a leader,  rcvcnt=%d >= twofp1=%d", s(id_), recvcnt, twofp1 );
		// state to D
		bool toDgood = trxnState_.goState( level_, trxnId, XIT_l );
		if ( toDgood ) {
			d("a331208 from XIT_l to toDgood true");
			// todo L2 L3
			t.setXit( XIT_m );
			t.setVoteInt( totalVotes_[trxnId] );
			strvec nullvec;
			pvec(otherLeaders);
			t.srvport = srvport_; 
			sstr dat; t.allstr(dat);
			//d("a33221 %s round-2 multicast otherLeaders trnmsg=[%s] ...", s(id_), s(dat) );
			multicast( otherLeaders, dat, false, nullvec );
			d("a33221 %s round-2 multicast otherLeaders done", s(id_));
		} else {
			d("a4457 %s XIT_l toDgood is false", s(id_));
		}
		collectTrxn_.erase(trxnId);
		totalVotes_.erase(trxnId);
	} else {
		// d("a2234 %s got XIT_l, a leader, but rcvcnt=%d < twofp1=%d, nostart round-2", s(id_), recvcnt, twofp1 );
		d("a2234 %s got XIT_l, a leader, but rcvcnt=%d < twofp1=%d", s(id_), recvcnt, twofp1 );
	}
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
	tryRecvM( beacon, trxnId, clientIP, sid, otherLeaders, followers, t);
}

void omserver::tryRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
					    const strvec &otherLeaders, const strvec &followers, OmicroTrxn &t )
{
	Byte curState;
	bool rc = trxnState_.getState(trxnId, curState);
	if ( rc && curState > ST_D ) {
		// later comers
		d("a70013 tryRecvL T_m curState=%c passed ST_D already, late msg, ignored", curState);
		return;
	}

	if ( rc && curState == ST_D ) {
		d("a81117 curState == ST_D doRecvM ...");
		doRecvM( beacon, trxnId, clientIP, sid, otherLeaders, followers, t );
	} else {
		// debug only
		++waitDCount_;
		if ( waitDCount_ > 100 ) {
			d("a9209 waitCount_ > 100 give up wait");
			i("E40430 error waitCount_ > 100 give up wait in tryRecvM");
			waitDCount_ = 0;
			return;
		}

		d("a81117 error curState != ST_D wait 100 millisecs then tryRecvM ...");
		timer2_->expires_at(timer2_->expiry() + boost::asio::chrono::milliseconds(100));
		timer2_->async_wait(boost::bind(&omserver::tryRecvM, this, beacon, trxnId, clientIP, sid, otherLeaders, followers, boost::ref(t) ));
	}
}

void omserver::doRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
					    const strvec &otherLeaders, const strvec &followers, OmicroTrxn &t )
{
	t.srvport = srvport_;

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
			t.srvport = srvport_;
			sstr dat; t.allstr(dat);
			omserver::multicast( followers, dat, false, nullvec );
			d("a33281 %s multicast XIT_n followers done", s(id_));

			// i do commit too to state F
			// block_.add( t );
			blockMgr_.saveTrxn( t );
			d("a9999 leader commit a TRXN %s ", s(trxnId));
			trxnState_.goState( level_, trxnId, XIT_n );  // to ST_F
			// reply back to client
			// d("a99992 conv=%ld clientconv=%ld\n", conv,  clientConv_[trxnId] );
			// sstr m( sstr("GOOD_TRXN|") + id_);
			// d("a4002 reply back good trxn...");
			// kcp_server_->send_msg(clientConv_[trxnId], m);
		} else {
			d("a72128 from XIT_m toEgood false");
		}
		collectTrxn_.erase(trxnId);
		totalVotes_.erase(trxnId);
	}

}

int omserver::multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec )
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
	for ( int i=0; i < len; ++i ) {
		NodeList::getData( hostVec[i], id, ip, port);
		thrdParam[i].srv = ip;
		thrdParam[i].port = atoi(port.c_str());
		srvport = ip + ":" + port;

		getPubkey( srvport, pubkey );
		OmicroTrxn t( trxnMsg.c_str() );
		t.makeSignature(pubkey);

		t.allstr(thrdParam[i].trxn);
		thrdParam[i].expectReply = expectReply;
		pthread_create(&thrd[i], NULL, &threadSendMsg, (void *)&thrdParam[i]);
	}

	// todo
	//sleep(5);

	int connected = 0;
	for ( int i=0; i < len; ++i ) {
		pthread_join( thrd[i], NULL );
		if ( expectReply ) {
			d("a59031 pthread_join i=%d done expectReply=1", i );
			if ( thrdParam[i].reply.size() > 0 ) {
				replyVec.push_back( thrdParam[i].reply );
			}
		}

		if ( thrdParam[i].reply != "NOCONN" ) {
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
    p->reply = cli.sendMessage( OM_RX,  p->trxn.c_str(), p->expectReply );
	d("a30114 cli.sendMessage returned p->expectReply=%d p->reply=[%s]", p->expectReply, s(p->reply) );
    return NULL;
}

