#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "essential/utility/strutil.h"
#include "util/ikcp.h"
//#include "client_with_asio.hpp"
#include "client_lib/kcp_client_util.h"
#include "client_lib/kcp_client_wrap.hpp"
#include "client.h"

int main(int argc, char* argv[])
{
	OmicroClient client( argv[1], atoi(argv[2]), 5 );
	std::string reply = client.sendMessage( "hihihihihi", 100 );
	std::cout << reply << std::endl;

	reply = client.sendMessage( "heloheelohello", 100 );
	std::cout << reply << std::endl;
}

