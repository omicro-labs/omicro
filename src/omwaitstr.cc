#include <time.h>
#include "omwaitstr.h"

OmWaitStr::OmWaitStr()
{
}

OmWaitStr::~OmWaitStr()
{
}

void OmWaitStr::add( const std::string & trxnId, const std::string &str )
{
	auto itr = tsStr_.find(trxnId);
	if ( itr == tsStr_.end() ) {
		time_t nowt = time(NULL);
		tsStr_.emplace( trxnId, std::make_pair(nowt, str) );
	} 
}

void OmWaitStr::get( const std::string & trxnId, std::string &str )
{
	auto itr = tsStr_.find(trxnId);
	if ( itr != tsStr_.end() ) {
		str = itr->second.second;
	}
}

void OmWaitStr::erase(const std::string & trxnId )
{
	tsStr_.erase(trxnId);
}

void OmWaitStr::cleanup(int keepSeconds )
{
	time_t  nowt = time(NULL);
	auto itr = tsStr_.begin();
	while ( itr != tsStr_.end() ) {
		if ( (nowt - itr->second.first) > keepSeconds ) {
			itr = tsStr_.erase( itr );
		} else {
			++itr;
		}
	}
}
