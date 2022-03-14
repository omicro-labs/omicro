#ifndef _dynamic_circuit_h_
#define _dynamic_circuit_h_

#include "omicrodef.h"

class nodeList;

class DynamicCircuit
{
  public:
  	DynamicCircuit();
  	~DynamicCircuit();
	void getZoneLeaders( const NodeList &nodeList, const sstr &beacon, strvec &vec );



  protected:
	void getLeaders( int numZones, const NodeList &nodeList, const sstr &beacon, strvec &vec );
};

#endif
