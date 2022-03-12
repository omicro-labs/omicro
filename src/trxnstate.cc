#include "trxnstate.h"

TrxnState::TrxnState()
{
}
TrxnState::~TrxnState()
{
}

bool TrxnState::getState( const sstr &trxnid, Byte &type, Byte &state )
{
	std::unordered_map<sstr, OmState>::iterator itr;
	itr = stateMap_.find( trxnid );
	if ( itr == stateMap_.end() ) {
		return false;
	} else {
		type = itr->second.type;
		state = itr->second.state;
		// transit = itr->second.transit;
		return true;
	}
}

bool TrxnState::goState( Byte level, const sstr &trxnid, Byte type, Byte transit )
{
	if ( level == 2 ) {
		return goStateL2( trxnid, type, transit );
	} else if ( level == 3 ) {
		return goStateL3( trxnid, type, transit );
	} else {
		return false;
	}

}

bool TrxnState::goStateL2( const sstr &trxnid, Byte type, Byte transit )
{
	OmState st;
	Byte nextState = ST_0;
	st.type = type;

	if ( transit == XIT_i ) {
		nextState = ST_A;
	} else if ( transit == XIT_j ) {
		nextState = ST_B;
	} else if ( transit == XIT_k ) {
		nextState = ST_C;
	} else if ( transit == XIT_l ) {
		nextState = ST_D;
	} else if ( transit == XIT_m ) {
		nextState = ST_E;
	} else if ( transit == XIT_n ) {
		nextState = ST_F;
	} else {
		return false;
	}

	st.state = nextState;
	insertOrUpdateState( trxnid, st );
	return true;
}

bool TrxnState::goStateL3( const sstr &trxnid, Byte type, Byte transit )
{
	OmState st;
	Byte nextState = ST_0;
	st.type = type;

	if ( transit == XIT_i ) {
		nextState = ST_A;
	} else if ( transit == XIT_j ) {
		nextState = ST_B;
	} else if ( transit == XIT_k ) {
		nextState = ST_C;
	} else if ( transit == XIT_l ) {
		nextState = ST_D;
	} else if ( transit == XIT_m ) {
		nextState = ST_E;
	} else if ( transit == XIT_n ) {
		nextState = ST_F;
	} else if ( transit == XIT_o ) {
		nextState = ST_G;
	} else if ( transit == XIT_p ) {
		nextState = ST_H;
	} else if ( transit == XIT_q ) {
		nextState = ST_I;
	} else {
		return false;
	}

	st.state = nextState;
	insertOrUpdateState( trxnid, st );
	return true;
}

void TrxnState::insertOrUpdateState( const sstr &trxnid, const OmState &st )
{
	std::unordered_map<sstr, OmState>::iterator itr;
	itr = stateMap_.find( trxnid );
	if ( itr == stateMap_.end() ) {
		stateMap_.emplace( trxnid, st );
	} else {
		// Terminate states cannot be active again
		if ( itr->second.state != ST_T ) {
			if ( itr->second != st ) {
				itr->second = st;
			}
		}
	}
}

void TrxnState::setState( const sstr &trxnid, Byte type, Byte state )
{
	OmState st { type, state };
	insertOrUpdateState( trxnid, st );
}

void TrxnState::terminateState( const sstr &trxnid, Byte type )
{
	OmState st { type, ST_T };
	insertOrUpdateState( trxnid, st );
}

void TrxnState::deleteState( const sstr &trxnid )
{
	stateMap_.erase( trxnid );
}


