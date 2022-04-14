#include <algorithm>
#define  RAPIDJSON_HAS_STDSTRING 1
#include "omjson.h"
#include "omicrodef.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

OmJson::OmJson()
{
}

OmJson::~OmJson()
{
}

static void jsonHandleLeaf( const rapidjson::Value &rv, const std::vector<sstr> &keep, const char *name,
				 			rapidjson::Writer<rapidjson::StringBuffer> &writer )
{
	bool exist = (keep.size() > 0) 
	             && ( std::find(keep.begin(), keep.end(), name ) != keep.end() );

    if ( rv.IsString() ) {
		if ( exist ) {
          	writer.String(rv.GetString());
		} else {
           	writer.String("");
		}
    } else if ( rv.IsInt() ) {
		if ( exist ) {
          	writer.Int(rv.GetInt());
		} else {
           	writer.Int(0);
		}
    } else if ( rv.IsUint() ) {
		if ( exist ) {
          	writer.Uint(rv.GetUint());
		} else {
           	writer.Uint(0);
		}
    } else if ( rv.IsInt64() ) {
		if ( exist ) {
          	writer.Int64(rv.GetInt64());
		} else {
           	writer.Int64(0);
		}
    } else if ( rv.IsUint64() ) {
		if ( exist ) {
          	writer.Uint64(rv.GetUint64());
		} else {
           	writer.Uint64(0);
		}
    } else if ( rv.IsDouble() ) {
		if ( exist ) {
          	writer.Double(rv.GetDouble());
		} else {
           	writer.Double(0);
		}
    } else if ( rv.IsBool() ) {
		if ( exist ) {
          	writer.Bool(rv.GetBool());
		} else {
           	writer.Bool( false );
		}
	} else {
        writer.String("");
	}
}

static void jsonRecCopy( const rapidjson::Value &dv,  const std::vector<sstr> &keep,
              			 rapidjson::Writer<rapidjson::StringBuffer> &writer )
{
    writer.StartObject();
    for ( auto& m : dv.GetObject() ) {
        writer.Key(m.name.GetString());
        const rapidjson::Value &v = m.value;

        if ( v.IsObject() ) {
            jsonRecCopy( v, keep, writer);
        } else if ( v.IsArray() ) {
            writer.StartArray();
            for ( rapidjson::SizeType i = 0; i < v.Size(); i++) {
                const rapidjson::Value &rv = v[i];
                if ( rv.IsObject() ) {
                    jsonRecCopy(rv, keep, writer);
                } else {
					//jsonHandleLeaf(rv, keep, m.name.GetString(), writer);
					// non-onjects inside array are dropped
                }
            }
            writer.EndArray();
        } else {
			jsonHandleLeaf(v, keep, m.name.GetString(), writer);
		}

    }
    writer.EndObject();
}

void OmJson::stripJson(const sstr &inJson,  const std::vector<sstr> &keep, sstr &outJson)
{
    rapidjson::Document dom;
    dom.Parse( inJson.c_str() );
    if ( dom.HasParseError() ) {
		return;
    }

    rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
    jsonRecCopy(dom, keep, writer);
	outJson = sbuf.GetString();
}

void OmJson::add(const std::string &key, const std::string &val )
{
	vec_.push_back( std::make_pair(key, val) );
}

void OmJson::json( std::string &str )
{
    rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
	writer.StartObject();
	for ( const auto &p : vec_ ) {
		writer.Key(p.first.c_str() );
		writer.String(p.second.c_str() );
	}
	writer.EndObject();
	str = sbuf.GetString();
}
