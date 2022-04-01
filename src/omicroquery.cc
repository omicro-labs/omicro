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

void OmicroQuery::str(sstr &data)
{
	data = sstr("QT|") + trnxId_;
}

void OmicroQuery::strGetPublicKey( sstr &pk )
{
	pk = sstr("QP|") + trnxId_;
}
