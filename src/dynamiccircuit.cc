
#include <math.h>
#include <iostream>
#include "nodelist.h"
#include "dynamiccircuit.h"
#include "xxHash/xxhash.h"
#include "omutil.h"
EXTERN_LOGGING


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
	uint len = nodeList_.size();
	uint dd = len/numZones;
	for ( unsigned int i=0; i < len; ++i ) {
		hash = XXH64( nodeList_[i].c_str(), nodeList_[i].size(), seed ) % len ;
		zone = hash / dd;
		if ( leader[zone].size() < 1 ) {
			leader[zone] = nodeList_[i];
		}
	}
	vec = leader;

	for ( int z = 0; z < numZones; ++z ) {
		const sstr &sr = leader[z];
		d("a83380 zone=%d leader=%s", z, s(sr));
	}

}

// L2
// return true if id is a leader
bool DynamicCircuit::getOtherLeaders( const sstr &beacon, const sstr &id, strvec &otherLeaders )
{
	int numZones = getNumZones();
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;

	uint len = nodeList_.size();
	uint dd = len/numZones;

	bool idIsLeader = false;
	for ( unsigned int i=0; i < len; ++i ) {
		const sstr &rec = nodeList_[i];
		hash = XXH64( s(rec), rec.size(), seed ) % len;
		zone = hash / dd;
		if ( leader[zone].size() < 1 ) {
			if ( id != rec ) {
				leader[zone] = rec;
			} else {
				idIsLeader = true;
			}
		}
	}

	for ( int i=0; i < numZones; ++i ) {
		if ( leader[i].size() > 0 ) {
			otherLeaders.push_back( leader[i] );
		}
	}

	return idIsLeader;
}


bool DynamicCircuit::isLeader( const sstr &beacon, const sstr &srvid, bool getFollowers, strvec &followersVec )
{
	int numZones = getNumZones();
	int seed = atoi( beacon.c_str() );

	uint len = nodeList_.size();
	uint dd = len/numZones;

	XXH64_hash_t hash;
	hash = XXH64( srvid.c_str(), srvid.size(), seed ) % len;
	int zoneid =  hash / numZones;
	d("a22201 isLeader check srvid=[%s] zoneid=%d", s(srvid), zoneid );

	int zone;
	bool isLeader = false;
	for ( unsigned int i=0; i < len; ++i ) {
		const sstr &rec = nodeList_[i];
		hash = XXH64( s(rec), rec.size(), seed ) % len;
		zone = hash / dd;
		if ( zone != zoneid ) {
			continue;
		}
		
		d("a21245 i=%d zone==zoneid=%d rec=[%s]", i, zone, s(rec));

		if ( isLeader == false ) {
			if ( srvid != rec ) {
				// first element in the same zone
				// not srvid, return false;
				d("a11023 rec=[%s] is not srvid=[%s] return false", s(rec), s(srvid) );
				return false;
			} else {
				isLeader = true;
				d("a11024 rec=[%s] is equal to srvid=[%s] isLeader=true, continue", s(rec), s(srvid) );
				continue; // skip leader
			}
		} 

		// add followers to vec
		if ( getFollowers ) {
			followersVec.push_back( rec );
		}
	}

	d("a33301 return isLeader=%d followersVec=%d", isLeader, followersVec.size() );
	return isLeader;
}


// L2
// return true if id is a leader
bool DynamicCircuit::getOtherLeadersAndFollowers( const sstr &beacon, const sstr &srvid, 
												  strvec &otherLeaders, strvec &followers )
{
	int numZones = getNumZones();
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;

	uint len = nodeList_.size();
	uint d = len/numZones;

	hash = XXH64( srvid.c_str(), srvid.size(), seed ) % len;
	int zoneid =  hash / numZones;

	bool idIsLeader = false;
	for ( unsigned int i=0; i < len; ++i ) {
		const sstr &rec  = nodeList_[i];
		hash = XXH64( rec.c_str(), rec.size(), seed ) % len;
		zone = hash / d;
		if ( leader[zone].size() < 1 ) {
			if ( srvid != rec ) {
				leader[zone] = rec;
			} else {
				idIsLeader = true;
			}
		}

		if ( zoneid == zone && srvid != rec  ) {
			followers.push_back( rec );
		}
	}

	for ( int i=0; i < numZones; ++i ) {
		if ( leader[i].size() > 0 ) {
			otherLeaders.push_back( leader[i] );
		}
	}

	return idIsLeader;
}
