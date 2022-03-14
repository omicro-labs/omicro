
#include <math.h>
#include <iostream>
#include "omicrodef.h"
#include "nodelist.h"
#include "dynamiccircuit.h"
#include "xxHash/xxhash.h"


DynamicCircuit::DynamicCircuit()
{
}

DynamicCircuit::~DynamicCircuit()
{
}

void DynamicCircuit::getZoneLeaders( const NodeList &nodeList, const sstr &beacon, strvec &vec )
{
	int nlen = nodeList.length();
	Byte layer = nodeList.getLayer();

	if ( layer == 2 ) {
		int numZones = int( sqrt(nlen) );
		getLeaders( numZones, nodeList, beacon, vec );
	} else {
		int numZones = int( std::cbrt(nlen) );
		getLeaders( numZones, nodeList, beacon, vec );
	}
}

void DynamicCircuit::getLeaders( int numZones, const NodeList &nodeList, const sstr &beacon, strvec &vec )
{
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;
	for ( unsigned int i=0; i < nodeList.list_.size(); ++i ) {
		hash = XXH64( nodeList.list_[i].c_str(), nodeList.list_[i].size(), seed );
		zone = hash / numZones;
		if ( leader[zone].size() < 1 ) {
			leader[zone] = nodeList.list_[i];
		}
	}
	vec = leader;

	for ( int z = 0; z < numZones; ++z ) {
		std::cout << "a83380 zone=" << z << " leader=" << leader[z] << std::endl;
	}

}
