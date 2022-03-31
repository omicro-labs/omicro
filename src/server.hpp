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

using boost::asio::ip::tcp;

struct ThreadParam
{
    sstr srv;
    int port;
    sstr trxn;
    sstr reply;
    bool expectReply;
};
void *threadSendMsg(void *arg);

class OmicroTrxn;

class omserver
{
  public:
    omserver(boost::asio::io_context &io_context, const sstr &srvip, const sstr &port);
	~omserver();

	void onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t);
	void onRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t);
	static int multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec );

	std::unordered_map<sstr, std::vector<uint>> collectTrxn_;
	std::unordered_map<sstr, ulong> totalVotes_;
	TrxnState trxnState_;
	NodeList nodeList_;
	int level_;
	sstr id_;

	// debug only
	sstr srvport_;
	int  waitCCount_;
	int  waitDCount_;
	// debug only

	BlockMgr  blockMgr_;
	sstr pubKey_;
	sstr secKey_;


  private:
    void do_accept();
	void readID();
	void readPubkey();
	void readSeckey();
	sstr getDataDir() const;
	void tryRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
				  const strvec &otherLeaders,  OmicroTrxn &t );
	void doRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
				  const strvec &otherLeaders,  OmicroTrxn &t );

	void tryRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
				  const strvec &otherLeaders, const strvec &followers, OmicroTrxn &t );
	void doRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
				  const strvec &otherLeaders,  const strvec &followers, OmicroTrxn &t );

	boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
	sstr address_, port_;
	btimer *timer1_; // ST_C
	btimer *timer2_; // ST_D


};

#endif 
