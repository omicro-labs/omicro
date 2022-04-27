#ifndef _om_client_pool_h_
#define _om_client_pool_h_

#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>
#include "omicroclient.h"

using CliPtr = std::shared_ptr<OmicroClient>;

class OmClientPool
{
  public:
  	OmClientPool();
  	~OmClientPool();

	CliPtr get(const std::string &srv, int port ); 
	void  erase( const std::string &srv, int port );

  protected:
    std::unordered_map<std::string, CliPtr> map_;
	std::mutex mutex_;
    
};

#endif
