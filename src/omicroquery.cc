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

void OmicroQuery::setSender( const sstr &from)
{
	sender = from;
}

void OmicroQuery::setTimeStamp( const sstr &ts)
{
	timestamp = ts;
}


void OmicroQuery::str(sstr &data)
{
	data = sstr("QT|") + trnxId_ + "|" + sender + "|" + timestamp;
}

void OmicroQuery::strGetPublicKey( sstr &pk )
{
	pk = sstr("QP|") + trnxId_;
}
