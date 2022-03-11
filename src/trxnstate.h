#ifndef _trxnstate_h_
#define _trxnstate_h_

#include <unordered_map>
#include "omicrodef.h"

// transit triggers (events)
#define  TRNS_START  0
#define  TRNS_X1     1
#define  TRNS_X2     2

// stages
#define  STG_ONE     1
#define  STG_TWO     2
#define  STG_THREE   3
#define  STG_FOUR    4

// states
#define  Z1     1
#define  Z2     2
#define  Z3     3
#define  Z4     4

struct OmState
{
	Byte  stage;
	Byte  state;
	Byte  transit;
};

class TrxnState
{
  public:
  	TrxnState();
  	~TrxnState();

	bool getState( const sstr &trxnid, Byte &stage, Byte &state, Byte &transit);
	void setState( const sstr &trxnid, Byte stage, Byte state, Byte transit);

  protected:
    std::unordered_map<sstr, OmState> stateMap_;
	// key: trxnid  value: state info
  	
};

#endif
