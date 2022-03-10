#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "essential/utility/strutil.h"
#include "util/ikcp.h"
#include "client_lib/kcp_client_util.h"
#include "client_lib/kcp_client_wrap.hpp"
#include "omicroclient.h"
#include "omicrotrxn.h"

int main(int argc, char* argv[])
{
	OmicroTrxn t1;
	OmicroClient client( argv[1], atoi(argv[2]), 5 );
	t1.makeDummyTrxn();
	t1.setInitTrxn();
	std::string reply = client.sendMessage( t1.str(), 100 );
	std::cout << reply << std::endl;

	OmicroTrxn t2;
	t2.makeDummyTrxn();
	//t2.setInitTrxn();
	reply = client.sendMessage( t2.str(), 100 );
	std::cout << reply << std::endl;
}

