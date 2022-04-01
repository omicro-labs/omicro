#ifndef _omicro_query_h_
#define _omicro_query_h_

#include "omicrodef.h"

class OmicroQuery
{
  public:
  	OmicroQuery();
  	~OmicroQuery();

	void setTrxnId( const sstr &id);
	void str( sstr &data);
	void strGetPublicKey( sstr &pk);

  protected:
  	sstr trnxId_;
};

#endif
