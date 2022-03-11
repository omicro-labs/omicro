#include "trxnstate.h"

TrxnState::TrxnState()
{
}
TrxnState::~TrxnState()
{
}

bool TrxnState::getState( const sstr &trxnid, Byte &stage, Byte &state, Byte &transit)
{
	std::unordered_map<sstr, OmState>::iterator itr;
	itr = stateMap_.find( trxnid );
	if ( itr == stateMap_.end() ) {
		return false;
	} else {
		stage = itr->second.stage;
		state = itr->second.state;
		transit = itr->second.transit;
		return true;
	}
}

void TrxnState::setState( const sstr &trxnid, Byte stage, Byte state, Byte transit)
{
	OmState st { stage, state, transit };

	std::unordered_map<sstr, OmState>::iterator itr;
	itr = stateMap_.find( trxnid );
	if ( itr == stateMap_.end() ) {
		stateMap_.emplace( trxnid, st );
	} else {
		itr->second = st;
	}
}
