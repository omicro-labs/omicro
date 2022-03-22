#include <chrono>
#include "omsync.h"
#include "omutil.h"

// -1 if timeout happened; 0: if flag turned true
int cv_wait_timeout( bool &flag, std::unique_lock<std::mutex> &lck, std::condition_variable &cv,  int sec )
{
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(sec) ;
    while( !flag )
    {
       auto res = cv.wait_until(lck, endTime);
       if (res == std::cv_status::timeout)
       {
            d("a3330 cv_wait_timeout timeout");
			return -1;
       }
    }
    d("a3330 cv cv_wait_timeout done flag=%d", flag);
	return 0;
}
