#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "g2logworker.h"
#include "g2log.h"
#include "server_lib/asio_kcp_log.hpp"
#include "server.hpp"
#include <muduo/base/Logging.h>
#include <muduo/base/LogFile.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/base/TimeZone.h>


int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 3)
        {
            std::cerr << "Usage: server <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    server 0.0.0.0 80\n";

            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    server 0::0 80\n";
            return 1;
        }

        system("mkdir asio_kcp_log");
        std::string path_to_log_file("./asio_kcp_log/");
        g2LogWorker logger(argv[0], path_to_log_file);
        g2::initializeLogging(&logger);
        AK_LOG(INFO) << "AK_LOG Server Start";

        LOG_INFO << "LOG_INFO server start";

        server serv(argv[1], argv[2]);

        serv.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
