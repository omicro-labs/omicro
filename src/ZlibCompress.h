#ifndef _om_zlib_compress_h_
#define _om_zlib_compress_h_

#include <string>
#include <zlib.h>

class ZlibCompress
{
  public:
	static void compress(const std::string& str, std::string &out, int compressionlevel = Z_BEST_COMPRESSION );

	static void uncompress(const std::string& str, std::string &out);

};

#endif
