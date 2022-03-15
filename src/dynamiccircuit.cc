
#include <math.h>
#include <iostream>
#include "omicrodef.h"
#include "nodelist.h"
#include "dynamiccircuit.h"
#include "xxHash/xxhash.h"


DynamicCircuit::DynamicCircuit(  const NodeList &nodeList )
    :nodeList_(nodeList)
{
	level_ = nodeList.getLevel();
}

DynamicCircuit::~DynamicCircuit()
{
}

int DynamicCircuit::getNumZones()
{
	int nlen = nodeList_.length();
	int numZones;
	if ( level_ == 2 ) {
		numZones = int( sqrt(nlen) );
	} else {
		numZones = int( std::cbrt(nlen) );
	}
	return numZones;
}

// L2 leaders
void DynamicCircuit::getZoneLeaders( const sstr &beacon, strvec &vec )
{
	int numZones = getNumZones();
	getLeaders( numZones, beacon, vec );
}

// L2 leaders
void DynamicCircuit::getLeaders( int numZones, const sstr &beacon, strvec &vec )
{
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;
	for ( unsigned int i=0; i < nodeList_.size(); ++i ) {
		hash = XXH64( nodeList_[i].c_str(), nodeList_[i].size(), seed );
		zone = hash / numZones;
		if ( leader[zone].size() < 1 ) {
			leader[zone] = nodeList_[i];
		}
	}
	vec = leader;

	for ( int z = 0; z < numZones; ++z ) {
		std::cout << "a83380 zone=" << z << " leader=" << leader[z] << std::endl;
	}

}

// L2
void DynamicCircuit::getOtherLeaders( const sstr &beacon, const sstr &id, strvec &otherLeaders )
{
	int numZones = getNumZones();
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;
	for ( unsigned int i=0; i < nodeList_.size(); ++i ) {
		hash = XXH64( nodeList_[i].c_str(), nodeList_[i].size(), seed );
		zone = hash / numZones;
		if ( leader[zone].size() < 1 && id != nodeList_[i] ) {
			leader[zone] = nodeList_[i];
		}
	}

	for ( int i=0; i < numZones; ++i ) {
		if ( leader[i].size() > 0 ) {
			otherLeaders.push_back( leader[i] );
		}
	}
}


bool DynamicCircuit::isLeader( const sstr &beacon, const sstr &srvid, strvec &vec )
{
	int numZones = getNumZones();
	int seed = atoi( beacon.c_str() );

	XXH64_hash_t hash;
	hash = XXH64( srvid.c_str(), srvid.size(), seed );
	int zoneid =  hash / numZones;

	int zone;
	bool isLeader = false;
	for ( unsigned int i=0; i < nodeList_.size(); ++i ) {
		hash = XXH64( nodeList_[i].c_str(), nodeList_[i].size(), seed );
		zone = hash / numZones;
		if ( zone != zoneid ) {
			continue;
		}

		if ( isLeader == false ) {
			if ( srvid != nodeList_[i] ) {
				// first element in the same zone
				// not srvid, return false;
				return false;
			} else {
				isLeader = true;
				continue; // skip leader
			}
		} 

		// add followers to vec
		vec.push_back( nodeList_[i] );
	}

	return isLeader;
}

