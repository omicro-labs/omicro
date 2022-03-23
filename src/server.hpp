#ifndef _OMSERVER_H_
#define _OMSERVER_H_

#include <boost/asio.hpp>
#include <string>
#include "nodelist.h"
#include "omicrodef.h"
#include "omutil.h"
#include "trxnstate.h"
#include "ommsghdr.h"

using boost::asio::ip::tcp;
using becode = boost::system::error_code;

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

	void onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t);
	void onRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t);
	static void multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec );


	std::unordered_map<sstr, std::vector<uint>> collectTrxn_;
	std::unordered_map<sstr, ulong> totalVotes_;
	TrxnState trxnState_;
	NodeList nodeList_;
	int level_;
	sstr id_;
	std::unordered_map<sstr, bool> passedC_;


  private:
    void do_accept();
	void readID();
	boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
	sstr getDataDir() const;
	std::mutex &getMutx(const sstr &trxnId);
	std::condition_variable &getCond( const sstr &trxnId);
	void clearMutxCond( const sstr &trxnId);

	sstr address_, port_;
	//std::unordered_map<sstr, std::vector<uint>> collectTrxn_;
	//std::unordered_map<sstr, ulong> totalVotes_;
	std::unordered_map<sstr, std::mutex> stmtx_;
	std::unordered_map<sstr, std::condition_variable> stcv_;
};

#endif 
