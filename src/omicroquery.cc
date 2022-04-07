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


void OmicroQuery::str( const sstr &qtype, sstr &data)
{
	//data = sstr("QT|") + trnxId_ + "|" + sender + "|" + timestamp;
	data = qtype + "|" + trnxId_ + "|" + sender + "|" + timestamp;
}

void OmicroQuery::strGetPublicKey( sstr &pk )
{
	pk = sstr("QP|") + trnxId_;
}
