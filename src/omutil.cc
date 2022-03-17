#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include "omutil.h"

EXTERN_LOGGING

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

void setLogFile(const char *fpath, bool append)
{
	FILE *fp;
	if ( append ) {
		fp = fopen(fpath, "a");
	} else {
		fp = fopen(fpath, "w");
	}

	if ( ! fp ) {
		printf("E11000 error open logfile [%s] to append, use stdout\n", fpath );
		fflush(stdout);
		g_loggingfp = stdout;
		return;
	}
	g_loggingfp = fp;
}

void log(FILE *f, const char * format, va_list args )
{
	char buf[32];
	time_t t= time(NULL);
	ctime_r(&t, buf);
	int len = strlen(buf);
	if ( buf[len-1] == '\n' ) {
		buf[len-1] = '\0';
	}

	g_log_mutex.lock();
	fprintf(f, "%s %d %ld: ", buf, getpid(), pthread_self() );
	vfprintf(f, format, args);
	fprintf(f, "\n");
	fflush(f);
	g_log_mutex.unlock();
}


void i(const char * format, ...)
{
	va_list(args);
	va_start(args, format);
	log(g_loggingfp,format, args);
	va_end(args);
}

void d(const char * format, ...)
{
	if ( ! g_debug ) return;
	va_list(args);
	va_start(args, format);
	log(g_loggingfp, format, args);
	va_end(args);
}

const char *s(const sstr &s, int sublen)
{
	if ( sublen <= 0 ) {
		return s.c_str();
	} else {
		return s.substr(0,sublen).c_str();
	}
}

void printvec( const strvec &vec )
{
	for ( const auto &r : vec ) {
		i("%s", s(r));
	}
}
