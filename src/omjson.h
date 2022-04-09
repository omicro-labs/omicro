#ifndef _om_json_h_
#define _om_json_h_

#include <vector>
#include <string>
//#include "omicrodef.h"

class OmJson
{
  public:
    OmJson();
    ~OmJson();

	static void stripJson(const std::string &inJson, const std::vector<std::string> &keep, 
						  std::string &outJson);
};

#endif
