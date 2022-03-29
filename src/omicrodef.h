#ifndef _omicro_def_h_
#define _omicro_def_h_

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>


using sstr = std::string;
using Byte = unsigned char;
using strvec = std::vector<std::string>;
using strshptr = std::shared_ptr<std::string>;
using bcode = boost::system::error_code;
using btimer = boost::asio::steady_timer;
using becode = boost::system::error_code;

#ifndef ulong
using ulong = unsigned long;
#endif

#ifndef uint
using uint = unsigned int;
#endif

#define INIT_LOGGING FILE *g_loggingfp = stdout; \
                     bool g_debug = false; \
					 std::mutex  g_log_mutex;

#define EXTERN_LOGGING extern FILE *g_loggingfp; \
                       extern bool g_debug; \
					   extern std::mutex g_log_mutex;
#endif
