#ifndef _trxnlist_h_
#define _trxnlist_h_

#include <string>
#include <stdio.h>

class TrxnList
{
  public:
    TrxnList();
    void setDataDir( const std::string &dataDir );
	~TrxnList();

	void saveTrxnList( const std::string &from, const std::string &timestamp );

  protected:
    std::string dataDir_;
    std::string fpath_;
	FILE  *fp_;
};

#endif
