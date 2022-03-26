#include <tchdb.h>
#include "omstore.h"
#include "omutil.h"
EXTERN_LOGGING

OmStore::OmStore( const char *fpath, int mode )
{
	ok_ = false;
	fpath_ = fpath;
	hdb_ = tchdbnew();
	if ( ! tchdbopen(hdb_, fpath, mode ) ) {
		d("E40283 error open [%s]", fpath );
		return;
	}

	ok_ = true;
}

OmStore::~OmStore()
{
	close();
}

int OmStore::put(const char *key, int ksize, const char *value, int vsize)
{
	bool rc = tchdbput(hdb_, (char*)key, ksize, (char*)value, vsize );
	if ( rc ) {
		d("a500278 OmStore::put data to [%s] OK", fpath_.c_str() );
		return 0;
	} else {
		i("E304032 OmStore::put data to [%s] error [%s]", fpath_.c_str(), lastError() );
		return -1; 
	}
}

// NULL or free it from caller
char *OmStore::get( const char *key )
{
	return (tchdbget2(hdb_, (char*)key) );
}

void OmStore::close()
{
	if ( ok_ && hdb_ ) {
		tchdbclose(hdb_);
		hdb_ = NULL;
		ok_ = false;
	}
}

const char * OmStore::lastError()
{
	if ( ! ok_ ) {
		return "";
	}

	int ecode = tchdbecode(hdb_);
	return tchdberrmsg(ecode);
}

