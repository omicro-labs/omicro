#ifndef _OMSESSION_H_
#define _OMSESSION_H_

#include <boost/asio.hpp>
#include <string>
#include "nodelist.h"
#include "omicrodef.h"
#include "omutil.h"
#include "trxnstate.h"
#include "ommsghdr.h"

using boost::asio::ip::tcp;
using becode = boost::system::error_code;

class OmicroTrxn;
class OmicroClient;
class OmServer;

class OmSession : public std::enable_shared_from_this<OmSession>
{
  public:
    OmSession( boost::asio::io_context& io_context, OmServer &srv, tcp::socket socket);
    void start();
	~OmSession();
  
  private:

    void do_read();
    void do_write(std::size_t length);
	void reply( const sstr &str, tcp::socket &sock );
	void doTrxnL2(const char *msg, int len);
	void doSimpleQuery(const char *msg, int len);
	void doQueryL2(const char *msg, int len);
	bool initTrxn( OmicroTrxn &txn );
	bool initQuery( OmicroTrxn &txn );
	void makeSessionID();
	bool validateTrxn( const sstr &trxnId, OmicroTrxn &txn, bool isInitTrxn, sstr &err );
	bool validateQuery( OmicroTrxn &txn, const sstr &trxnId, bool isInitTrxn, sstr &err );
	void getQueryResult( const sstr &trxnId, const sstr &sender, sstr &res );
  
	boost::asio::io_context& io_context_;
	OmServer &serv_;

    tcp::socket socket_;
    char hdr_[OMHDR_SZ+1];
	bool stop_;
	sstr clientIP_;

	sstr  sid_;

};

#endif
