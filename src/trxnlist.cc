#include "trxnlist.h"
#include "omutil.h"
#include "omlog.h"
#include <sys/stat.h>
EXTERN_LOGGING

TrxnList::TrxnList()
{
	fp_ = NULL;
}

TrxnList::~TrxnList()
{
	if ( fp_ ) {
		fclose( fp_ );
	}
}

void TrxnList::setDataDir( const std::string &dataDir )
{
	dataDir_ = dataDir;
	sstr dpath = dataDir_ + "/blocks";
	::mkdir( dpath.c_str(), 0700 );
	fpath_ = dpath + "/blocklist";
	fp_ = fopen(fpath_.c_str(), "a");
	if ( ! fp_ ) {
		i("E32061 error open [%s] append mode", s(fpath_) );
	}
}

void TrxnList::saveTrxnList( const std::string &from, const std::string &timestamp )
{
	d("a33001 saveTrxnList from=[%s]", s(from) );
	if ( ! fp_ ) {
		i("E32028 saveTrxnList fp NULL return");
		return;
	}

	fprintf(fp_, "%s~%s|", timestamp.c_str(), from.c_str() );
	d("a35028 saveTrxnList fpath_=[%s]", fpath_.c_str());
	fflush(fp_);
}
