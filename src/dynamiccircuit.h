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
	bool isLeader( const sstr &beacon, const sstr &srvid,  bool getFollowers, strvec &followers  );
	bool getOtherLeaders( const sstr &beacon, const sstr &id, strvec &vec );
	bool getOtherLeadersAndFollowers( const sstr &beacon, const sstr &id, strvec &leadvec, strvec &followers );

  protected:
  	int level_;
  	const NodeList &nodeList_;
  	int getNumZones();
	void getLeaders( int numZones, const sstr &beacon, strvec &vec );
};

#endif
