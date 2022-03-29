#ifndef _om_store_h_
#define _om_store_h_

#include <tchdb.h>
#include "omicrodef.h"

#define OM_DB_WRITE ( HDBOWRITER|HDBOCREAT )
#define OM_DB_READ  (HDBOREADER )

class OmStore
{
  public:
    OmStore( const char *fpath, int mode );
    ~OmStore();

	int put(const char *key, int ksize, const char *value, int vsize);

	// NULL or free by caller
	char *get( const char *key );

	void close();
	const char *lastError();

  protected:
    bool ok_;
	TCHDB *hdb_; 
	sstr fpath_;
};

#endif
