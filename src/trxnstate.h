#ifndef _trxnstate_h_
#define _trxnstate_h_

#include <unordered_map>
#include "omicrodef.h"

// state type
#define  STYPE_L2   'A'  // layer2 trxn
#define  STYPE_L3   'B'  // layer3 trxn
#define  STYPE_EN   'E'  // enrollement
#define  STYPE_RW   'R'  // reward
#define  STYPE_IP   'I'  // change IP
#define  STYPE_DE   'D'  // delist a node


// transit triggers (events)
#define  XIT_i    'i' 
#define  XIT_j    'j' 
#define  XIT_k    'k' 
#define  XIT_l    'l' 
#define  XIT_m    'm' 
#define  XIT_n    'n' 
#define  XIT_o    'o' 
#define  XIT_p    'p' 
#define  XIT_q    'q' 

// states
#define  ST_0   '0'
#define  ST_A   'A'
#define  ST_B   'B'
#define  ST_C   'C'
#define  ST_D   'D'
#define  ST_E   'E'
#define  ST_F   'F'
#define  ST_G   'G'
#define  ST_H   'H'
#define  ST_I   'I'
#define  ST_J   'J'
#define  ST_T   'T'


struct OmState
{
	Byte  type;
	Byte  state;
	//Byte  transit;
	bool operator==( const OmState &st ) {
		if ( type == st.type && state == st.state ) {
			return true;
		}
		return false;
	}
	bool operator!=( const OmState &st ) {
		if ( type != st.type || state != st.state ) {
			return true;
		}
		return false;
	}
};

class TrxnState
{
  public:
  	TrxnState();
  	~TrxnState();

	bool goState( Byte level, const sstr &trxnid, Byte type, Byte transit );
	bool getState( const sstr &trxnid, Byte &type, Byte &state );
	void setState( const sstr &trxnid, Byte type, Byte state );
	void deleteState( const sstr &trxnid );
	void terminateState( const sstr &trxnid, Byte type );

  protected:
    std::unordered_map<sstr, OmState> stateMap_;
	// key: trxnid  value: state info

	bool goStateL2( const sstr &trxnid, Byte type, Byte transit);
	bool goStateL3( const sstr &trxnid, Byte type, Byte transit);
	void insertOrUpdateState( const sstr &trxnid, const OmState &st );
  	
};

#endif
