#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "../essential/utility/strutil.h"
#include "client_with_asio.hpp"
#include "../client_lib/kcp_client_wrap.hpp"


void* thread_func( void *ptr);

using boost::asio::ip::tcp;

enum { max_length = 1024 };

void test_kcp(boost::asio::io_service &io_service, const int port_bind_to, const char* ip, const int port, size_t test_msg_size)
{
    client_with_asio client(io_service, port_bind_to, std::string(ip), port, test_msg_size);
    io_service.run();
}



/*** old 
int main(int argc, char* argv[])
{
    try
    {
        if (argc != 5)
        {
            std::cerr << "Usage: asio_kcp_client <port_bind_to> <connect_to_host> <connect_to_port> <test_msg_lenth>\n";
            std::cerr << "asio_kcp_client 22222 232.23.223.1 12345 500\n";
            return 1;
        }

        boost::asio::io_service io_service;
        test_kcp(io_service, std::atoi(argv[1]), argv[2], std::atoi(argv[3]), std::atoi(argv[4]));
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

***/

#if 0
// 1000: 12 seconds
int main(int argc, char* argv[])
{
	int cnt = 1000;  // 12 seconds  connnect sendmsg;  connect only: 12 seconds
	                 // 5 seconds connect and sendmsg  after KCP_UPDATE_INTERVAL==1 ms in client_lib
	int i;
	int suc = 0;
	for(i=0; i < cnt; ++i )
	{
		asio_kcp::kcp_client_wrap client;
		int rc = client.connect(23494, argv[1], atoi(argv[2]) );
		//int rc = client.connect(0, argv[1], atoi(argv[2]) );
		if ( rc < 0 ) {
			printf("error connect\n");
			continue;
		}

		++suc;

		std::string msg = ".start hi there, this is hthejej ejfjdhfj djhfdhfjdhfj dhfjdh fjdhfjd jfhdj fjdhfjd fjdh fjhd jfhdj fjdhf d9djfkdjfkdjf djfdhjfdjend.";
		client.send_msg( msg );
		client.stop();
	}
	printf("success=%d/totalcnt=%d messages sent\n", suc, cnt );
	fflush(stdout);
}
#endif


#if 0
int main(int argc, char* argv[])
{
	int cnt = 10000;
	int i;
	int suc = 0;

		asio_kcp::kcp_client_wrap client;
		//int rc = client.connect(23490, argv[1], atoi(argv[2]) );
		int rc = client.connect(0, argv[1], atoi(argv[2]) );
		if ( rc < 0 ) {
			printf("error connect\n");
			exit(1);
		}

		++suc;

		std::string msg = ".start hi there, this is hthejej ejfjdhfj djhfdhfjdhfj dhfjdh fjdhfjd jfhdj fjdhfjd fjdh fjhd jfhdj fjdhf d9djfkdjfkdjf djfdhjfdjend.";
	for(i=0; i < cnt; ++i )
	{
		client.send_msg( msg );
	}
	// 10000 == 0.02 seconds  fast
	client.stop();
	printf("%d messages sent\n", cnt );
	fflush(stdout);
}
#endif

class PP {
   public:
      std::string serv;
	  int  port;
	  bool send;
};

// 1000: 12 seconds
int main(int argc, char* argv[])
{
	int cnt = 1000;  // 12 seconds  connnect sendmsg;  connect only: 12 seconds
	                 // 5 seconds connect and sendmsg  after KCP_UPDATE_INTERVAL==1 ms in client_lib
	int i;
	int suc = 0;
	int ret;
	PP pp;
	pp.serv = argv[1];
	pp.port = atoi(argv[2]);
	pp.send = true;

	pthread_t thread[cnt];

	for(i=0; i < cnt; ++i )
	{
		ret = pthread_create(&thread[i], NULL, &thread_func, (void*)&pp);
		if ( ret != 0 ) {
			printf("i=%d create thread error skip\n", i );
			fflush(stdout);
			continue;
		}
		// detach
		//pthread_detach( thread[i] );
	}

	for(i=0; i < cnt; ++i )
	{
		pthread_join( thread[i], NULL );
	}

	printf("success=%d/totalcnt=%d messages sent\n", suc, cnt );
	fflush(stdout);
}

void* thread_func( void *ptr)
{
	PP *pp = (PP*)ptr;
	if ( ! pp->send ) return NULL;

	asio_kcp::kcp_client_wrap client;
	int rc = client.connect(0, pp->serv.c_str(), pp->port, 10 );
	if ( rc < 0 ) {
		return NULL;
	}

	std::string msg = ".start hi there, this is hthejej ejfjdhfj djhfdhfjdhfj dhfjdh";
	msg += "line2  hi there, this is hthejej ejfjdhfj djhfdhfjdhfj dhfjdh fjdhfjd jfhdj";
	msg += "line3  hi there, this is hthejej ejfjdhfj djhfdhfjdhfj dhfjdh fjdhfjd jfhdjend.";
	client.send_msg( msg );
	client.stop();
	return NULL;
}

