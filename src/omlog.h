#ifndef _omicro_log_h_
#define _omicro_log_h_

#include <mutex>

#define INIT_LOGGING FILE *g_loggingfp = stdout; \
                     bool g_debug = false; \
					 std::mutex  g_log_mutex;

#define EXTERN_LOGGING extern FILE *g_loggingfp; \
                       extern bool g_debug; \
					   extern std::mutex g_log_mutex;
#endif
