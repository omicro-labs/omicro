#include "server.hpp"
#include <boost/bind.hpp>
#include <signal.h>
#include <cstdlib>
#include <iostream>
#include "essential/utility/strutil.h"
#include "omicrotrxn.h"

OmicroServer::OmicroServer(const sstr& address, const sstr& port)
    : io_service_(),
    signals_(io_service_),
    stopped_(false),
    kcp_server_(io_service_, address, port),
    test_timer_(io_service_),
	address_(address), port_(port)
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(boost::bind(&OmicroServer::handle_stop, this));

    kcp_server_.set_callback(
        std::bind(&OmicroServer::event_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    hook_test_timer();
}

void OmicroServer::run()
{
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    io_service_.run();
}

void OmicroServer::handle_stop()
{
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    kcp_server_.stop();
    stopped_ = true;
}

void OmicroServer::event_callback(kcp_conv_t conv, kcp_svr::eEventType event_type, std::shared_ptr<sstr> msg)
{
    std::cout << "event_callback:" << conv << " type:" << kcp_svr::eventTypeStr(event_type) << "msg: " << *msg << std::endl;
    if (event_type == kcp_svr::eRcvMsg)
    {
        // auto send back msg for testing.
		// qwer
		std::shared_ptr<sstr> m1 = std::make_shared<sstr>("Server echo back: hello from server");
        kcp_server_.send_msg(conv, m1);
        //kcp_server_.send_msg(conv, msg);

		OmicroTrxn t(msg->c_str());
		bool validTrxn = t.isValidClientTrxn();
		if ( ! validTrxn ) {
			std::shared_ptr<sstr> m = std::make_shared<sstr>("INVALID_CLIENT_TRXN");
        	kcp_server_.send_msg(conv, m);
			return;
		}

		bool isInitTrxn = t.isInitTrxn();
		if ( isInitTrxn ) {
			std::shared_ptr<sstr> m = std::make_shared<sstr>("Your initiated a new trxn. Working on it ...");
        	kcp_server_.send_msg(conv, m);
			initTrxn( conv, t );
		} else {
		}
    }
}

// qwer
bool OmicroServer::initTrxn( kcp_conv_t conv, OmicroTrxn &txn )
{
	// find zone leaders and ask them to collect votes from members
	// nodeList_ is std::vector<string>
	sstr beacon = txn.getBeacon();
	
	return true;
}


void OmicroServer::hook_test_timer(void)
{
    if (stopped_)
        return;

    test_timer_.expires_from_now(boost::posix_time::milliseconds(10000));
    test_timer_.async_wait(std::bind(&OmicroServer::handle_test_timer, this));
}

void OmicroServer::handle_test_timer(void)
{
    //std::cout << "."; std::cout.flush();
    hook_test_timer();

    //test_force_disconnect();
}

void OmicroServer::test_force_disconnect(void)
{
    static kcp_conv_t conv = 1000;
    kcp_server_.force_disconnect(conv);
    conv++;
}

sstr OmicroServer::getDataDir()
{
	sstr dir;
	dir = "../data/";
	dir += address_ + "/" + port_;
	return dir;

}
