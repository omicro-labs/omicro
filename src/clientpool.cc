#include "clientpool.h"

OmClientPool::OmClientPool()
{
}

OmClientPool::~OmClientPool()
{
}

CliPtr OmClientPool::get(const std::string &srv, int port )
{
	const std::lock_guard<std::mutex> lock(mutex_);
	std::string id = srv + std::to_string(port);

	auto itr = map_.find( id );
	if ( itr == map_.end() ) {
		CliPtr ptr = std::make_shared<OmicroClient>(srv.c_str(), port );
		map_.emplace(id, ptr );
		return ptr;
	} else {
		return itr->second;
	}
}

void OmClientPool::erase( const std::string &srv, int port )
{
	const std::lock_guard<std::mutex> lock(mutex_);
	std::string id = srv + std::to_string(port);
	map_.erase( id );
}
