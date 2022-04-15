#ifndef _om_json_h_
#define _om_json_h_

#include <vector>
#include <string>

class OmJson
{
  public:
    OmJson();
    ~OmJson();

	static void stripJson(const std::string &inJson, const std::vector<std::string> &keep, 
						  std::string &outJson);

	void add(const std::string &key, const std::string &val );
	void json( std::string &str );
	static void getObjStr( const std::string &name, const std::string &jsonArr, std::string &resultObj );

  protected:
	std::vector<std::pair<std::string,std::string>> vec_;
};

#endif
