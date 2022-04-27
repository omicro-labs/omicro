/*
 * Copyright (C) Omicro Authors
 *
 * Omicro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Omicro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the LICENSE file. If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <boost/asio.hpp>
#include "server.hpp"
#include "omutil.h"

INIT_LOGGING

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc < 3) {
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

	while ( true ) {
		try {
        	boost::asio::io_context io_context;
        	OmServer s(io_context, argv[1], argv[2]);
        	io_context.run();
		} catch (std::exception& e) {
        	i("E44308 exception: [%s] i am %s %s", e.what(), argv[1], argv[2] );
    	}
    }

    return 0;
}

