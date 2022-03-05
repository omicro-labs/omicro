#include <pthread.h>
#include <string>
#include "client.h"
#include "client_lib/kcp_client_wrap.hpp"

class ThreadParam {
  public:
      std::string serv;
	  int  port;
      std::string msg;
};

int sendMessageToHost( const char *host, int port, const std::string &msg)
{
	ThreadParam pp;
	pp.serv = host;
	pp.port = port;
	pp.msg = msg;
	
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, &clientThreadFunc, (void*)&pp);
	if ( rc != 0 ) {
		return -1;
	}
	pthread_detach( thread );
	return 0;
}


void* clientThreadFunc( void *ptr)
{
    ThreadParam *pp = (ThreadParam*)ptr;
    asio_kcp::kcp_client_wrap client;
    int rc = client.connect(0, pp->serv.c_str(), pp->port );
    if ( rc < 0 ) {
        return NULL;
    }

    client.send_msg( pp->msg );
    client.stop();
    return NULL;
}



