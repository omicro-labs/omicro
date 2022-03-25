#include <tchdb.h>
#include "omstore.h"
#include "omutil.h"

omstore::omstore( const char *fpath, int mode )
{
	ok_ = false;
	hdb_ = tchdbnew();
	if ( ! tchdbopen(hdb_, "casket.tch", mode ) ) {
		d("E40283 error open [%s]", fpath );
		return;
	}

	ok_ = true;
}

omstore::~omstore()
{
	close();
}

int omstore::put(const char *key, int ksize, const char *value, int vsize)
{
	bool rc = tchdbput(hdb_, (char*)key, ksize, (char*)value, vsize );
	if ( rc ) {
		return 0;
	} else {
		return -1; 
	}
}

// NULL or free it from caller
char *omstore::get( const char *key )
{
	return (tchdbget2(hdb_, (char*)key) );
}

void omstore::close()
{
	if ( ok_ && hdb_ ) {
		tchdbclose(hdb_);
		hdb_ = NULL;
		ok_ = false;
	}
}

const char * omstore::lastError()
{
	if ( ! ok_ ) {
		return "";
	}

	int ecode = tchdbecode(hdb_);
	return tchdberrmsg(ecode);
}

