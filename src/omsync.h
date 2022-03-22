#ifndef _om_sync_h_
#define _om_sync_h_

#include <mutex>
#include <condition_variable>

int cv_wait_timeout( bool &flag, std::unique_lock<std::mutex> &lck, std::condition_variable &cv,  int sec );

#endif
