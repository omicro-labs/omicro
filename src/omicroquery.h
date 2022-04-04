#ifndef _omicro_query_h_
#define _omicro_query_h_

#include "omicrodef.h"

class OmicroQuery
{
  public:
  	OmicroQuery();
  	~OmicroQuery();

	void setTrxnId( const sstr &id);
	void setSender( const sstr &from);
	void setTimeStamp( const sstr &ts);
	void str( sstr &data);
	void strGetPublicKey( sstr &pk);

  protected:
  	sstr trnxId_;
  	sstr sender;
  	sstr timestamp;
};

#endif
