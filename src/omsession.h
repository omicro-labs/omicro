#ifndef _OMSESSION_H_
#define _OMSESSION_H_

#include <boost/asio.hpp>
#include <string>
//#include <boost/noncopyable.hpp>
#include "nodelist.h"
#include "omicrodef.h"
#include "omutil.h"
#include "trxnstate.h"
#include "ommsghdr.h"

using boost::asio::ip::tcp;
using becode = boost::system::error_code;

class OmicroTrxn;
class OmicroClient;
class omserver;

class omsession : public std::enable_shared_from_this<omsession>
{
  public:
    omsession( boost::asio::io_context& io_context, omserver &srv, tcp::socket socket);
    void start();
  
  private:

    void do_read();
    void do_write(std::size_t length);
	void reply( const sstr &str );
	void callback(const sstr &msg);
	bool initTrxn( OmicroTrxn &txn );
	void makeSessionID();
  
	boost::asio::io_context& io_context_;
	omserver &serv_;

    tcp::socket socket_;
    char hdr_[OMHDR_SZ+1];
	bool stop_;
	sstr clientIP_;

	sstr  sid_;
};

#endif // _SERVER_HPP
