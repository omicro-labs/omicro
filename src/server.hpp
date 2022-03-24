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
	~omserver();

	void onRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t);

	void onRecvM( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, OmicroTrxn &t);
	static void multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec );

	std::unordered_map<sstr, std::vector<uint>> collectTrxn_;
	std::unordered_map<sstr, ulong> totalVotes_;
	TrxnState trxnState_;
	NodeList nodeList_;
	int level_;
	sstr id_;


  private:
    void do_accept();
	void readID();
	boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
	sstr getDataDir() const;
	void tryRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
				  const strvec &otherLeaders,  OmicroTrxn &t );
	void doRecvL( const sstr &beacon, const sstr &trxnId, const sstr &clientIP, const sstr &sid, 
				  const strvec &otherLeaders,  OmicroTrxn &t );

	sstr address_, port_;
	boost::asio::steady_timer *timer_;

};

#endif 
