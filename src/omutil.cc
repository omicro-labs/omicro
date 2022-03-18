#include <sys/time.h>
#include <time.h>
#include <stdio.h>
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
    char tmstr[48];
    struct tm result;
    struct timeval now;
    gettimeofday( &now, NULL );
	time_t tsec = now.tv_sec;
	int ms = now.tv_usec / 1000;
    gmtime_r( &tsec, &result ); 
    strftime( tmstr, sizeof(tmstr), "%Y-%m-%d %H:%M:%S", &result );
	char msb[5];
	sprintf(msb, ".%03d", ms);
	strcat( tmstr, msb );

	g_log_mutex.lock();
	fprintf(f, "%s %d %ld: ", tmstr, getpid(), pthread_self()%10000 );
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

const char *s(const sstr &str)
{
	return str.c_str();
}

void pvec( const strvec &vec )
{
	for ( const auto &r : vec ) {
		i("%s", s(r));
	}
}

void dpvec( const strvec &vec )
{
	for ( const auto &r : vec ) {
		printf("%s\n", s(r));
		fflush(stdout);
	}
}

