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
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

OmicroSimpleQuery::OmicroSimpleQuery()
{
}
OmicroSimpleQuery::~OmicroSimpleQuery()
{
}

void OmicroSimpleQuery::setTrxnId( const sstr &id )
{
	trnxId_ = id;
}

void OmicroSimpleQuery::setSender( const sstr &from)
{
	sender = from;
}

void OmicroSimpleQuery::setTimeStamp( const sstr &ts)
{
	timestamp = ts;
}


void OmicroSimpleQuery::str( const sstr &qtype, sstr &data)
{
	//data = qtype + "|" + trnxId_ + "|" + sender + "|" + timestamp;
 	rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
    writer.StartObject();

    writer.Key("QT");
    writer.String( qtype.c_str() );

    writer.Key("TID");
    writer.String( trnxId_.c_str() );

    writer.Key("FRM");
    writer.String( sender.c_str() );

    writer.Key("TS");
    writer.String( timestamp.c_str() );

    writer.EndObject();
	data = sbuf.GetString();
}

void OmicroSimpleQuery::strGetPublicKey( sstr &pk )
{
	// pk = sstr("QP|") + trnxId_;
 	rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
    writer.StartObject();

    writer.Key("QT");
    writer.String("QP");

    writer.Key("TID");
    writer.String( trnxId_.c_str() );

    writer.Key("FRM");
    writer.String( sender.c_str() );

    writer.Key("TS");
    writer.String( timestamp.c_str() );

    writer.EndObject();
	pk = sbuf.GetString();
}
