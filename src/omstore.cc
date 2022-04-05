/*
 * Copyright (C) Omicro Authors
 *
 * Omicro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Omicro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the LICENSE file. If not, see <http://www.gnu.org/licenses/>.
 */
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
		d("a500278 OmStore::put data to [%s] OK ksize=%d vsize=%d", fpath_.c_str(), ksize, vsize );
		d("a500278 put key=[%s] value=[%s]", key, value );
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

