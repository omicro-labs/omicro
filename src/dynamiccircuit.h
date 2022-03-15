#ifndef _dynamic_circuit_h_
#define _dynamic_circuit_h_

#include "omicrodef.h"

class NodeList;

class DynamicCircuit
{
  public:
  	DynamicCircuit( const NodeList &nodeList );
  	~DynamicCircuit();
	void getZoneLeaders( const sstr &beacon, strvec &vec );
	bool isLeader( const sstr &beacon, const sstr &srvid,  strvec &followers  );
	void getOtherLeaders( const sstr &beacon, const sstr &id, strvec &vec );

  protected:
  	int level_;
  	const NodeList &nodeList_;
  	int getNumZones();
	void getLeaders( int numZones, const sstr &beacon, strvec &vec );
};

#endif
