#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include "nodelist.h"
#include "omicrodef.h"
#include "omutil.h"
#include "trxnstate.h"
#include "ommsghdr.h"

using boost::asio::ip::tcp;
using becode = boost::system::error_code;

class OmicroTrxn;
class OmicroClient;

struct ThreadParam
{
	sstr srv;
	int port;
	sstr trxn;
	sstr reply;
	bool expectReply;
};

void *threadSendMsg(void *arg);

class omsession : public std::enable_shared_from_this<omsession>
{
  public:
    omsession( boost::asio::io_context& io_context, sstr id, int level, const NodeList &nodeL, tcp::socket socket) ;
    void start();
  
  private:
	boost::asio::io_context& io_context_;
    void do_read();
    void do_write(std::size_t length);
	void reply( const sstr &str );
	void multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec);
	void callback(const sstr &msg);
	bool initTrxn( OmicroTrxn &txn );
	void makeSessionID();
  
    tcp::socket socket_;
    char hdr_[OMHDR_SZ+1];
	bool stop_;
	sstr id_;
	int  level_;
	const NodeList &nodeList_;
	TrxnState trxnState_;
	sstr clientIP_;
	bool passedC_;

	std::mutex stmtx_;
	std::condition_variable stcv_;
	sstr  sid_;
};

/// The signal_set is used to register for process termination notifications.
// boost::asio::signal_set signals_;

class omserver
{
  public:
    omserver(boost::asio::io_context &io_context, const sstr &srvip, const sstr &port);

  private:
    void do_accept();
	void readID();
	boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
	sstr getDataDir() const;

	int level_;
	NodeList nodeList_;
	sstr address_, port_;
	sstr id_;
};

#endif // _SERVER_HPP
