#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <boost/asio.hpp>
//#include <boost/bind.hpp>
#include "server.hpp"
#include "omutil.h"

INIT_LOGGING

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc < 3)
        {
            i("Usage: server <address> <port>");
            return 1;
        }

		if ( argc >= 4 ) {
			if ( 0 == strcmp(argv[3], "debug" ) ) {
				g_debug = true;
			}
		}

		sstr logf = sstr("../log/omserver_") + argv[1] + "_" +  argv[2];
		setLogFile(s(logf), false);
		i("***** omicroserver start %s %s *****\n", argv[1], argv[2]);

        boost::asio::io_context io_context;
        omserver s(io_context, argv[1], argv[2]);
        io_context.run();
    }
    catch (std::exception& e)
    {
        i("E44308 exception: [%s] i am %s %s, exit", e.what(), argv[1], argv[2] );
    }

    return 0;
}

