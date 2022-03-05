#ifndef _omicro_client_h_
#define _omicro_client_h_

void* clientThreadFunc( void *ptr);
int sendMessageToHost( const char *host, int port, const std::string &msg );

#endif
