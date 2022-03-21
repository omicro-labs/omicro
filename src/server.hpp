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
    omsession(sstr id, int level, const NodeList &nodeL, tcp::socket socket) ;
    void start();
  
  private:
    void do_read();
    void do_write(std::size_t length);
	void reply( const sstr &str );
	void multicast( const strvec &hostVec, const sstr &trxnMsg, bool expectReply, strvec &replyVec);
	void callback(const sstr &msg);
	bool initTrxn( OmicroTrxn &txn );
  
    tcp::socket socket_;
    //enum { max_length = 3024 };
    char hdr_[OMHDR_SZ+1];
    // char data_[max_length];
	bool stop_;
	sstr id_;
	int  level_;
	const NodeList &nodeList_;
	TrxnState trxnState_;
};

/// The signal_set is used to register for process termination notifications.
// boost::asio::signal_set signals_;

class omserver
{
  public:
    omserver(boost::asio::io_context& io_context, sstr srvip, sstr port)
        :acceptor_(io_context, tcp::endpoint(boost::asio::ip::address::from_string(srvip.c_str()), atoi(port.c_str()) ))
    {
		address_ = srvip;
		port_ = port;
		readID();
		level_ = nodeList_.getLevel();
        do_accept();
    }

  private:
    void do_accept();
	void readID();
    tcp::acceptor acceptor_;
	sstr getDataDir() const;

	int level_;
	NodeList nodeList_;
	sstr address_, port_;
	sstr id_;
};

#endif // _SERVER_HPP
