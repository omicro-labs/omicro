#ifndef _om_query_h_
#define _om_query_h_

#include <vector>
#include "omicrodef.h"

class OmQuery
{
  public:
    OmQuery();
    ~OmQuery();

	void str( sstr &qstr );
	void addField( const sstr &field );
	sstr predicate_;
	std::vector<sstr> fields_;

};

#endif
