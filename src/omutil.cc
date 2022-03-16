#include <sys/time.h>
#include "omutil.h"

unsigned int onefplus1( int N )
{
	int f = (N-1)/3;
	return f + 1;
}

unsigned int twofplus1( int N )
{
	int f = (N-1)/3;
	return 2*f + 1;
}

void nowTime( time_t &sec, time_t &msec )
{
    struct timeval now;
    gettimeofday( &now, NULL );
	sec = now.tv_sec;
	msec = now.tv_usec / 1000;
}

unsigned long getNowTimeUS()
{
    struct timeval now;
    gettimeofday( &now, NULL );
    return (now.tv_sec*1000000 + now.tv_usec);
}
