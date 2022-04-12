#ifndef _om_token_h_
#define _om_token_h_

#include <string>
#include <vector>
#include <unordered_set>

class OmToken {
  public:
    static void getMintJson( const std::vector<std::string> &vec, std::string &json );
    static void getXferJson( const std::vector<std::string> &vec, std::string &json );
	static void getKVPairs( const std::string &token, 
						    std::vector<std::pair<std::string,std::string>> &kvVec );

	static bool hasDupNames( const std::string &json1, const std::string &json2 );
	static void getKeysSet( const std::string &token, std::unordered_set<std::string> &set );
	static bool findKeyInSet( const std::string &token, const std::unordered_set<std::string> &set );
};

#endif
