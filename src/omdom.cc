#define  RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include "omdom.h"

OmDom::OmDom( const std::string &json )
{
	valid_ = true;
	dom_.Parse( json );
	if ( dom_.HasParseError() ) {
		valid_ = false;
		return;
	}
}

OmDom::~OmDom()
{
}

void OmDom::get(const std::string &key, std::string &val )
{
	if ( ! valid_ ) {
		return;
	}

	rapidjson::Value::ConstMemberIterator itr = dom_.FindMember( key.c_str() );
	if ( itr == dom_.MemberEnd() ) {
		val = "";
		return;
	} 

	//printf("%s\n", itr->value.GetType();
	//printf("%s\n", itr->value.GetString());
	//Value o(kObjectType);
	//Value a(kArrayType);

	// const rapidjson::Value& rv = dom_[ key.c_str() ];
	const rapidjson::Value& rv = itr->value;
	if ( rv.IsString() ) {
		val = rv.GetString();
		return;
	}

	char buf[32];

    if ( rv.IsInt() ) {
		sprintf(buf, "%d", rv.GetInt() );
		val = buf;
    } else if ( rv.IsUint() ) {
		sprintf(buf, "%u", rv.GetUint() );
		val = buf;
    } else if ( rv.IsInt64() ) {
		sprintf(buf, "%ld", rv.GetInt64() );
		val = buf;
    } else if ( rv.IsUint64() ) {
		sprintf(buf, "%lu", rv.GetUint64() );
		val = buf;
    } else if ( rv.IsDouble() ) {
		sprintf(buf, "%f", rv.GetDouble() );
		val = buf;
	} else {
		val = "";
	}

}
