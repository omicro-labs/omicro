#ifndef _om_util_h_
#define _om_util_h_

#include <time.h>
#include <stdio.h>
#include "omicrodef.h"

unsigned int onefplus1( int N );
unsigned int twofplus1( int N );
void nowTime( time_t &sec, time_t &msec );
unsigned long getNowTimeUS();

void setLogFile(const char *fpath, bool append=false);

void i(const char * format, ...);
void d(const char * format, ...);
void log(FILE *f, const char * format, va_list args);

const char *s(const sstr &str);
void pvec( const strvec &vec );
void dpvec( const strvec &vec );

ssize_t saferead( int fd, char *buf, ulong len );
ssize_t safewrite( int fd, const char *buf, ulong len );




#endif
