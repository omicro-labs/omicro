#include <boost/bind.hpp>
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

EXTERN_LOGGING
using namespace boost::asio::ip;

omsession::omsession(sstr id, int level, const NodeList &nodeL, tcp::socket socket)
        : socket_(std::move(socket)), nodeList_(nodeL)
{
    stop_ = false;
    id_ = id;
    level_ = level;
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
                      callback( msg );
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

    /***
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
    ***/

	d("a53550 in omsession::reply str=[%s] len=%d", s(str), str.size() );
	d("a00291 server write header back hdr_=[%s] ...", hdr_ );
    int len1 = boost::asio::write(socket_, boost::asio::buffer(hdr_, OMHDR_SZ) );
	d("a00291 server write header back hdr_=[%s] done sentbytes=%d", hdr_, len1 );

	d("a00292 server write str back str=[%s] ...", str.c_str() );
   	int len2 = boost::asio::write(socket_, boost::asio::buffer(str.c_str(), str.size()));
	d("a00292 server write str back str=[%s] strlen=%d done sentbytes=%d", str.c_str(), str.size(), len2 );
}

void omsession::callback(const sstr &msg)
{
	d("a2108 callback: msg=[%s]", msg.substr(0, 40).c_str());

	// auto send back msg for testing.
	/**
	sstr m1 = sstr("Server echo back: hello from server ") + id_;
	d("a0223848 Server echo bac m1.size=%d", m1.size() );
	reply( m1 );
	**/

	OmicroTrxn t(msg.c_str());
	bool validTrxn = t.isValidClientTrxn();
	if ( ! validTrxn ) {
		sstr m(sstr("BAD_INVALID_TRXN|") + id_ + "|" + msg);
		reply(m);
		return;
	}

	sstr trxnId = t.getTrxnID();
	bool isInitTrxn = t.isInitTrxn();
	bool rc;
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
					d("a31112 %s multicast followers for vote expect reply ..", s(id_));
					pvec( followers );
					multicast( followers, t.str(), true, replyVec );
					d("a31112 %s multicast followers for vote done replyVec=%d", s(id_), replyVec.size() );

					// got replies from followers, state to C
					bool toCgood = trxnState_.goState( level_, trxnId, XIT_k );
					if ( toCgood ) {
						d("a55550 toCgood true");
						if ( level_ == 2 ) {
							int votes = replyVec.size(); // how many replied
							t.setVoteInt( votes );
							// send el-'l' xit to other leaders
							t.setXit( XIT_l );
							strvec otherLeaders;
							circ.getOtherLeaders( beacon, id_, otherLeaders );
							d("a31102 %s round-1 multicast otherLeaders noreplyexpected ..", s(id_));
							pvec(otherLeaders);
							multicast( otherLeaders, t.str(), false, replyVec );
							d("a31102 %s round-1 multicast otherLeaders done replyVec=%d", s(id_), replyVec.size() );
						} else {
							// level_ == 3  todo
							d("a63311 error level_ == 2 false");
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
			d("a5501 received XIT_j reply back good");
			sstr m(sstr("GOOD_TRXN|XIT_j|")+id_);
			reply(m);
		} else if ( xit == XIT_l ) {
			d("a92822 %s received XIT_l ...", s(id_) );

			sstr m = sstr("GOOD_TRXN|XIT_l|")+id_ + "upon receiving XIT_l";
			reply(m);
			d("a38221 after recv XIT_l replied back [%s]", s(m) );

		    // received one XIT_l, there may be more XIT_l in next 3 seconds
			static std::unordered_map<sstr, std::vector<uint>> collectTrxn;
			static std::unordered_map<sstr, ulong> totalVotes;

			strvec otherLeaders;
			DynamicCircuit circ( nodeList_);
			bool iAmLeader = circ.getOtherLeaders( beacon, id_, otherLeaders );
			if ( iAmLeader ) {
				collectTrxn[trxnId].push_back(1);
				totalVotes[trxnId] += t.getVoteInt();
				d("a3733 got XIT_l am leader increment collectTrxn[trxnId]");

				uint twofp1 = twofplus1( otherLeaders.size() + 1);
				d("a53098 twofp1=%d otherleaders=%d", twofp1, otherLeaders.size() );
				uint recvcnt = collectTrxn[trxnId].size();
				if ( recvcnt >= twofp1 ) { 
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
					} else {
						d("a4457 %s toDgood is false", s(id_));
					}
					collectTrxn.erase(trxnId);
					totalVotes.erase(trxnId);
				} else {
					d("a2234 %s got XIT_l, a leader, but rcvcnt=%d < twofp1=%d, nostart round-2", s(id_), recvcnt, twofp1 );
				}
			} else {
				d("a2234 %s got XIT_l, i am not leader", s(id_) );
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
						sstr m( sstr("GOOD_TRXN|") + id_);
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

	d("a555023 callback done");
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
#endif

/*** server
***/

void omserver::do_accept()
{
    acceptor_.async_accept(
          [this](bcode ec, tcp::socket socket)
          {
            if (!ec)
            {
                std::shared_ptr<omsession> sess = std::make_shared<omsession>(id_, level_, nodeList_, std::move(socket));
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

