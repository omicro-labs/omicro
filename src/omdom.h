#ifndef _om_dom_h_
#define _om_dom_h_

#include <string>
#include <rapidjson/document.h>
/**
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
**/

class OmDom
{
  public:
    OmDom( const std::string &str );
    ~OmDom();
	void get(const std::string &key, std::string &val );

  protected:
  	rapidjson::Document dom_;
	bool valid_;

};

#endif
