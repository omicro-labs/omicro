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

sstr&& OmicroQuery::str()
{
	return std::move(sstr("QT|") + trnxId_);
}

sstr&& OmicroQuery::strGetPublicKey()
{
	return std::move(sstr("QP|") + trnxId_);
}
