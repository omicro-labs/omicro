#ifndef _dynamic_circuit_h_
#define _dynamic_circuit_h_

#include "omicrodef.h"

using strvec = std::vector<std::string>;

class NodeList;

class DynamicCircuit
{
  public:
  	DynamicCircuit( const NodeList &nodeList );
  	~DynamicCircuit();
	void getZoneLeaders( const sstr &beacon, strvec &vec );
	bool isLeader( const sstr &beacon, const sstr &srvid,  bool getFollowers, strvec &followers  );
	bool getOtherLeaders( const sstr &beacon, const sstr &id, strvec &vec );
	bool getOtherLeadersAndThisFollowers( const sstr &beacon, const sstr &id, strvec &leaders, strvec &followers );
	bool getLeader( const sstr &beacon, const sstr &id, strvec &leader );

  protected:
  	int level_;
  	const NodeList &nodeList_;
  	int getNumZones();
  	int getNumFullZones();
	int getZoneSize();
	void getLeaders( int numZones, int numFullZones, const sstr &beacon, strvec &vec );
};

#endif
