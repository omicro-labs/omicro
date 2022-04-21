#ifndef _OMSERVER_H_
#define _OMSERVER_H_

#include <boost/asio.hpp>
#include <string>
#include <random>
#include "nodelist.h"
#include "omicrodef.h"
#include "omutil.h"
#include "trxnstate.h"
#include "ommsghdr.h"
#include "blockmgr.h"
#include "omwaitcount.h"
#include "omwaitstr.h"
#include "omtimer.h"

using boost::asio::ip::tcp;

struct ThreadParam
{
    sstr srv;
    int port;
    sstr trxn;
    sstr reply;
	char msgType;
    bool expectReply;
};
void *threadSendMsg(void *arg);

class OmicroTrxn;

class omserver
{
  public:
    omserver(boost::asio::io_context &io_context, const sstr &srvip, const sstr &port);
	~omserver();

	void onRecvK( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t);
	void onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t);
	void onRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn t);

	int multicast( char msgType, const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec );
	void getPubkey( const sstr &srvport, sstr &pubkey );
	void addQueryVote( const sstr &trxnId, int votes );
	void getQueryVote( const sstr &trxnId, int &votes );

	OmWaitCount  collectKTrxn_;
	OmWaitCount  totalKVotes_;

	OmWaitCount  collectLTrxn_;
	OmWaitCount  totalLVotes_;

	OmWaitCount  collectMTrxn_;
	OmWaitCount  totalMVotes_;


	std::unordered_map<sstr, sstr> srvport_pubkey_;

	OmWaitCount queryVote_;
	OmWaitStr   qResult_;

	TrxnState trxnState_;
	NodeList nodeList_;
	int level_;
	sstr id_;

	// debug only
	sstr srvport_;

	BlockMgr  blockMgr_;
	sstr pubKey_;
	sstr secKey_;
	sstr address_, port_;


  private:
    void do_accept();
	void readID();
	void readPubkey();
	void readSeckey();
	void readSrvportPubkey();
	sstr getDataDir() const;

	void doCleanup();

	boost::asio::io_context &io_context_;
	//OmTimer  cTimer_;
    tcp::acceptor acceptor_;

	btimer *cleanupTimer_; // cleanup cached trxn items

};

#endif 
