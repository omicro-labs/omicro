#ifndef _om_util_h_
#define _om_util_h_

#include <time.h>

unsigned int onefplus1( int N );
unsigned int twofplus1( int N );
void nowTime( time_t &sec, time_t &msec );
unsigned long getNowTimeUS();

#endif
