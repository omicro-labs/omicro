#include "trxnstate.h"
#include "omutil.h"

TrxnState::TrxnState()
{
}
TrxnState::~TrxnState()
{
}

bool TrxnState::getState( const sstr &trxnid, Byte &state )
{
	OmStateItr itr;
	itr = stateMap_.find( trxnid );
	if ( itr == stateMap_.end() ) {
		return false;
	} else {
		state = itr->second.state;
		return true;
	}
}

bool TrxnState::goState( Byte level, const sstr &trxnid, Byte transit )
{
	Byte curState;
	OmStateItr itr;
	itr = stateMap_.find( trxnid );
	if ( itr == stateMap_.end() ) {
		curState = ST_0;
	} else {
		curState = itr->second.state;
	}

	if ( level == 2 ) {
		return goStateL2( trxnid, transit, curState, itr );
	} else if ( level == 3 ) {
		return goStateL3( trxnid, transit, curState, itr );
	} else {
		return false;
	}

}

// prevXitc == ST_0 meaning trxnid does not exist in stateMap_
bool TrxnState::goStateL2( const sstr &trxnid, Byte transit, Byte curState, OmStateItr itr )
{
	OmState st;
	Byte nextState;

	if ( curState == ST_0 && transit == XIT_i ) {
		nextState = ST_A;
	} else if ( curState == ST_A && transit == XIT_j ) {
		nextState = ST_B;
	} else if ( curState == ST_B && transit == XIT_k ) {
		nextState = ST_C;
	} else if ( curState == ST_C && transit == XIT_l ) {
		nextState = ST_D;
	} else if ( curState == ST_D && transit == XIT_m ) {
		nextState = ST_E;
	} else if ( curState == ST_E && transit == XIT_n ) {
		nextState = ST_F;
	} else {
		d("a62330 TrxnState::goStateL2 curState=[%c] transit=[%c] not allowed", char(curState), char(transit) );
		return false;
	}

	st.state = nextState;
	insertOrUpdateState( trxnid, st, itr );
	return true;
}

bool TrxnState::goStateL3( const sstr &trxnid, Byte transit, Byte curState, OmStateItr itr )
{
	OmState st;
	Byte nextState;

	if ( curState == ST_0 && transit == XIT_i ) {
		nextState = ST_A;
	} else if ( curState == ST_A && transit == XIT_j ) {
		nextState = ST_B;
	} else if ( curState == ST_B && transit == XIT_k ) {
		nextState = ST_C;
	} else if ( curState == ST_C && transit == XIT_l ) {
		nextState = ST_D;
	} else if ( curState == ST_D && transit == XIT_m ) {
		nextState = ST_E;
	} else if ( curState == ST_E && transit == XIT_n ) {
		nextState = ST_F;
	} else if ( curState == ST_F && transit == XIT_o ) {
		nextState = ST_G;
	} else if ( curState == ST_G && transit == XIT_p ) {
		nextState = ST_H;
	} else if ( curState == ST_H && transit == XIT_q ) {
		nextState = ST_I;
	} else {
		return false;
	}

	st.state = nextState;
	insertOrUpdateState( trxnid, st, itr );
	return true;
}

void TrxnState::insertOrUpdateState( const sstr &trxnid, const OmState &st, OmStateItr itr )
{
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

void TrxnState::setState( const sstr &trxnid, Byte state )
{
	OmStateItr itr = stateMap_.find( trxnid );
	OmState st { state };
	insertOrUpdateState( trxnid, st, itr );
}

void TrxnState::terminateState( const sstr &trxnid)
{
	OmStateItr itr = stateMap_.find( trxnid );
	OmState st { ST_T };
	insertOrUpdateState( trxnid, st, itr );
}

void TrxnState::deleteState( const sstr &trxnid )
{
	stateMap_.erase( trxnid );
}


