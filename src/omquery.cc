#include "omquery.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>


OmQuery::OmQuery()
{
}

OmQuery::~OmQuery()
{
}

void OmQuery::addField( const sstr &field )
{
	fields_.push_back(field);
}

void OmQuery::str( sstr &qstr )
{
 	rapidjson::StringBuffer bs;
    rapidjson::Writer<rapidjson::StringBuffer> writer(bs);
    writer.StartObject();
    writer.Key("predicate");
    writer.String( predicate_.c_str() );

    writer.Key("fields");
    writer.StartArray();
	for ( auto& f : fields_ ) {
		writer.String( f.c_str() );
	}
    writer.EndArray();
    writer.EndObject();
	qstr = (char*)bs.GetString();
}
