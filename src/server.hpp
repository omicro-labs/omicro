#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include "server_lib/server.hpp"
#include "nodelist.h"
#include "omicrodef.h"
#include "trxnstate.h"

class OmicroTrxn;

struct ThreadParam
{
	sstr srv;
	int port;
	sstr trxn;
	sstr reply;
};

void *threadSendMsg(void *arg);


class OmicroServer : private boost::noncopyable
{
    public:
        /// Construct the server to listen on the specified TCP address and port
        explicit OmicroServer(const sstr& address, const sstr &port);

        /// Run the server's io_service loop.  Must set_callback first then call run. Do not change callback after run.
        void run();

    private:
        /// Handle a request to stop the server.
        void handle_stop();

        void event_callback(kcp_conv_t conv, kcp_svr::eEventType event_type, std::shared_ptr<sstr> msg);

        void hook_test_timer(void);
        void handle_test_timer(void);
        void test_force_disconnect(void);
		sstr getDataDir();
		bool initTrxn( kcp_conv_t conv, OmicroTrxn &txn );
		void readID();
		void multicast( const strvec &hostVec, const sstr &trxnMsg, strvec &replyVec );


    private:
        /// The io_service used to perform asynchronous operations.
        boost::asio::io_service io_service_;

        /// The signal_set is used to register for process termination notifications.
        boost::asio::signal_set signals_;

        bool stopped_;

        /// The connection manager which owns all live connections.
        kcp_svr::server kcp_server_;

        boost::asio::deadline_timer test_timer_;

		sstr address_;
		sstr port_;
		NodeList nodeList_;
		TrxnState  trxnState_;
		sstr id_;
		int  level_;
};

#endif // _SERVER_HPP
