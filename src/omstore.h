#ifndef _om_store_h_
#define _om_store_h_

#include <tchdb.h>

#define OM_DB_WRITE ( HDBOWRITER|HDBOCREAT )
#define OM_DB_READ  (HDBOREADER )

class omstore
{
  public:
    omstore( const char *fpath, int mode );
    ~omstore();

	int put(const char *key, int ksize, const char *value, int vsize);
	char *get( const char *key );
	void close();
	const char *lastError();

  protected:
    bool ok_;
	TCHDB *hdb_; 
};

#endif
