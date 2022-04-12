#include "omicrodef.h"
#include "omtoken.h"
#include "omlimits.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

// vec is array of key-val pairs "name: petnft, max: 1, url: http://abceruxv123"
// name and max are required. Users choose to add any other arbitrary properties.
// return: json "[ { ...}, { ... }, {....} ]";
void OmToken::getMintJson( const std::vector<sstr> &vec, sstr &json )
{
	if ( vec.size() < 1 ) return;
	if ( vec.size() > 128 ) return; // todo; each token costs 1 O-token

	rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
	writer.StartArray();

	std::vector<std::pair<sstr,sstr>> kvVec;
	bool hasName = false;
	bool hasMax = false;
	bool hasIn = false;
	bool hasOut = false;
	bool hasBal = false;

	sstr maxVal;
	for (const auto &s: vec ) {
		writer.StartObject();
		kvVec.clear();
		OmToken::getKVPairs(s, kvVec);
		if ( kvVec.size() > OM_MAX_TOKEN_PROPS ) {
			return;
		}

		for ( const auto& pair: kvVec) {
			writer.Key( pair.first.c_str() );
			writer.String( pair.second.c_str() );
			if ( pair.first == "name" ) {
				hasName = true;
			} else if ( pair.first == "max" ) {
				hasMax = true;
				maxVal = pair.second;
			} else if ( pair.first == "in" ) {
				hasIn = true;
			} else if ( pair.first == "out" ) {
				hasOut = true;
			} else if ( pair.first == "bal" ) {
				hasBal = true;
			}
		}

		if ( hasIn || hasOut || hasBal ) { return; }

		writer.Key( "bal" ); 
		writer.String( maxVal.c_str() );

		writer.Key( "in" );
		writer.String( "0" );

		writer.Key( "out" );
		writer.String( "0" );
		writer.EndObject();
	}
	if ( hasIn || hasOut ) {
		return;
	}

	if ( hasName && hasMax ) {
		writer.EndArray();
		json = sbuf.GetString();
	}
}

// token: "name: aaa, max: 123 , url: http://uurfjfppsvb, prop1: ffk, prop2: eieied"
// key-values in the string cannot contain whitespaces
// key-val has no double quotes
void OmToken::getKVPairs( const sstr &token, 
					      std::vector<std::pair<sstr,sstr>> &kvVec )
{
    sstr key, val;
    const char *p = token.c_str();
    while ( *p == ' ' || *p == '\t' ) ++p; // q at ,
    if ( *p == '\0' ) return;
    const char *q = p;
    while ( *p != '\0' ) {
        while ( *q != ':' && *q != '\0' ) ++q;
        if ( *q == '\0' ) {
			break;
		}
        key = sstr(p, q-p);
        ++q;  // pass ':'
        while ( *q == ' ' || *q == '\t' ) ++q;
        if ( *q == '\0' ) {
			break;
		}
        p = q;
        while ( *q != ' ' && *q != '\t' && *q != ',' && *q != '\0' ) ++q; // q at ,
        val = sstr(p, q-p);
		kvVec.push_back( std::make_pair(key, val) );
        while ( *q == ' ' || *q == '\t' || *q == ',' ) ++q;
        //if ( *q == '\0' ) break;
        //while ( *q == ' ' || *q == '\t' || *q == ',' ) ++q; 
        p = q;
    }
} 

// json1: "[{ "name": "xzs", ...}, {name: dsds, ...}, {...}]"
// json2: "[{ name: xzs, ...}, {name: tok2, ...}, {...}]"
// key-val has double quotes
bool OmToken::hasDupNames( const sstr &json1, const sstr &json2 )
{
	if ( json1.size() < 1 || json2.size() < 1 ) return false;
	std::unordered_set<sstr> set;
	OmToken::getKeysSet( json1, set );
	bool found = OmToken::findKeyInSet( json2, set );
	return found;
}


// key-val has double quotes
void OmToken::getKeysSet( const sstr &token, std::unordered_set<sstr> &set )
{
    sstr key, val;
    const char *p = token.c_str();
    while ( *p == ' ' || *p == '\t' || *p == '{' || *p == '[' ) {
		++p; // q at ,
	}
	//printf("a335 p=[%s]\n", p );
    if ( *p == '\0' ) return;
    const char *q = p;
    while ( *p != '\0' ) {
        while ( *q != '"' && *q != '\0' ) ++q;
        if ( *q == '\0' ) { break; }

		++q;
		p = q;
        if ( *p == '\0' ) { break; }
        while ( *q != '"' && *q != '\0' ) ++q;
        if ( *q == '\0' ) { break; }

        key = sstr(p, q-p);
		set.emplace(key);
		//printf("a13010 find key=[%s]\n", key.c_str() );
        ++q;  // pass second '"'

        while ( *q != ',' && *q != '\0' ) ++q;
        if ( *q == '\0' ) { break; }
		++q;
		//printf("a3012 q=[%s]\n", q );
        p = q;
    }
} 

// key-val has double quotes
bool OmToken::findKeyInSet( const sstr &token, const std::unordered_set<sstr> &set )
{
    sstr key, val;
    const char *p = token.c_str();
    while ( *p == ' ' || *p == '\t' || *p == '{' || *p == '[' ) {
		++p; // q at ,
	}
    if ( *p == '\0' ) return false;
	//printf("a332 p=[%s]\n", p );
    const char *q = p;
    while ( *p != '\0' ) {
        //while ( *q != ':' && *q != '\0' ) ++q;
        while ( *q != '"' && *q != '\0' ) ++q;
        if ( *q == '\0' ) { break; }
		++q;
		p = q;
        if ( *p == '\0' ) { break; }
        while ( *q != '"' && *q != '\0' ) ++q;
        if ( *q == '\0' ) { break; }

        key = sstr(p, q-p);
		//vec.push_back( key );
		//printf("a30312 find key=[%s]\n", key.c_str() );
		if ( set.find(key) != set.end() ) {
			return true;
		}
        ++q;  // pass second '"'
        while ( *q != ',' && *q != '\0' ) ++q;
        if ( *q == '\0' ) { break; }
		++q;
		//printf("a3012 q=[%s]\n", q );
        p = q;
    }

	return false;
} 

// vec is array of key-val pairs "name: petnft, amount: 1"
// name is required. default of amount is 1
// return: json "[ { ...}, { ... }, {....} ]";
void OmToken::getXferJson( const std::vector<sstr> &vec, sstr &json )
{
	if ( vec.size() < 1 ) return;
	if ( vec.size() > OM_MAX_TOKEN_XFER ) return; 

	rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
	writer.StartArray();

	std::vector<std::pair<sstr,sstr>> kvVec;
	bool hasName = false;

	sstr maxVal;
	for (const auto &s: vec ) {
		writer.StartObject();
		kvVec.clear();
		OmToken::getKVPairs(s, kvVec);
		if ( kvVec.size() > 4 ) {
			return;
		}

		hasName = false; 
		for ( const auto& pair: kvVec) {
			writer.Key( pair.first.c_str() );
			writer.String( pair.second.c_str() );
			if ( pair.first == "name" ) {
				hasName = true;
			} 
		}

		if ( ! hasName ) { return; }
		writer.EndObject();
	}

	writer.EndArray();
	json = sbuf.GetString();
}
