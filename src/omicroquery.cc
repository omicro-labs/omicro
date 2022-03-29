#include "omicrodef.h"
#include "omicroquery.h"

OmicroQuery::OmicroQuery()
{
}
OmicroQuery::~OmicroQuery()
{
}

void OmicroQuery::setTrxnId( const sstr &id )
{
	trnxId_ = id;
}

sstr OmicroQuery::str()
{
	return sstr("QT|") + trnxId_;
}
