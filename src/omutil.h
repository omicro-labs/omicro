#ifndef _om_util_h_
#define _om_util_h_

#include <time.h>
#include <stdio.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include "omicrodef.h"
#include "omstrsplit.h"


// f+1
unsigned int onefplus1( int N );

// 2f+1
unsigned int twofplus1( int N );

// now seconds and milliseconds
void nowTime( time_t &sec, time_t &msec );

// now in microseconds
unsigned long getNowTimeUS();

// set log file path
void setLogFile(const char *fpath, bool append=false);

// info log
void i(const char * format, ...);

// dug log
void d(const char * format, ...);

// log function
void log(FILE *f, const char * format, va_list args);

const char *s(const sstr &str);
// print a vector
void pvec( const strvec &vec );

// debug print a vector
void dpvec( const strvec &vec );

// try to read all bytes of len
long saferead( int fd, char *buf, long len );

// try to write all bytes of len
long safewrite( int fd, const char *buf, long len );

// integer power
ulong ipow(ulong num, int power);

// similar to mkdir -p <fpath>
void makedirPath( const sstr &fullpath );

#endif
