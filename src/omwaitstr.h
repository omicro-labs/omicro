#ifndef _om_wait_str_h_
#define _om_wait_str_h_

#include <string>
#include <unordered_map>

class OmWaitStr
{
  public:
    OmWaitStr();
    ~OmWaitStr();

	void add( const std::string & trxnId, const std::string &str );
	void get( const std::string & trxnId, std::string &str );
	void erase(const std::string & trxnId );

	void cleanup(int keepSeconds );

  protected:
    std::unordered_map<std::string, std::pair<time_t, std::string> > tsStr_;

};

#endif
