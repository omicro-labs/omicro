/*
 * Copyright (C) Omicro Authors
 *
 * Omicro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Omicro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the LICENSE file. If not, see <http://www.gnu.org/licenses/>.
 */
#include <math.h>
#include "nodelist.h"
#include "dynamiccircuit.h"
#include "xxHash/xxhash.h"
#include "omutil.h"
#include "omlog.h"
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
		if ( nlen > numZones*numZones ) {
			++numZones;
		}
	} else {
		numZones = int( std::cbrt(nlen) );
		if (  nlen > numZones*numZones*numZones ) {
			++numZones;
		}
	}
	return numZones;
}

int DynamicCircuit::getNumFullZones()
{
	int nlen = nodeList_.length();
	int numZones;
	d("a09821 getNumFullZones nlen=%d", nlen );
	if ( level_ == 2 ) {
		numZones = int( sqrt(nlen) );
		d("a09821 level=2 getNumFullZones numZones=%d", numZones );
	} else {
		numZones = int( std::cbrt(nlen) );
		d("a09821 notlevel=2  level=%d getNumFullZones numZones=%d", level_, numZones );
	}
	return numZones;
}

// L2 leaders
void DynamicCircuit::getZoneLeaders( const sstr &beacon, strvec &vec )
{
	int numZones = getNumZones();
	int numFullZones = getNumFullZones();
	getLeaders( numZones, numFullZones, beacon, vec );
}

// L2 leaders
void DynamicCircuit::getLeaders( int numZones, int numFullZones,  const sstr &beacon, strvec &vec )
{
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;
	uint len = nodeList_.size();

	uint dd = len/numFullZones;
	d("a93930 getLeaders numZones=%d numFullZones=%d nodeList_.size=%d dd=%d", numZones, numFullZones, len, dd );

	for ( unsigned int i=0; i < len; ++i ) {
		hash = XXH64( nodeList_[i].c_str(), nodeList_[i].size(), seed ) % len ;
		zone = hash / dd;
		d("a3333 zone=%d", zone );
		if ( leader[zone].size() < 1 ) {
			leader[zone] = nodeList_[i];
			d("a22337 zone=%d leader[%d] = [%s]", zone, zone, nodeList_[i].c_str() );
		}
	}
	vec = leader;

	/***
	for ( int z = 0; z < numZones; ++z ) {
		const sstr &sr = leader[z];
		d("a83380 zone=%d leader=%s", z, s(sr));
	}
	***/

}

// L2
// return true if id is a leader
bool DynamicCircuit::getOtherLeaders( const sstr &beacon, const sstr &srvid, strvec &otherLeaders )
{
	int numZones = getNumZones();
	strvec leader(numZones);
	int seed = atoi( beacon.c_str() );
	int zone;

	uint len = nodeList_.size();
	int numFullZones = getNumFullZones();
	uint dd = len/numFullZones;

	XXH64_hash_t hash;
	hash = XXH64( srvid.c_str(), srvid.size(), seed ) % len;
	int zoneid =  hash / dd;
	d("a20231 getOtherLeaders srvid=[%s] zoneid=%d dd=%d", s(srvid), zoneid, dd );

	bool idIsLeader = false;
	bool first = true;
	for ( unsigned int i=0; i < len; ++i ) {
		const sstr &rec = nodeList_[i];
		hash = XXH64( s(rec), rec.size(), seed ) % len;
		zone = hash / dd;

		if ( zone == zoneid ) {
			d("a33080 getOtherLeaders zone == zoneid =%d first=%d rec=[%s] =?= srvid=[%s]", zone, first, s(rec), s(srvid) );
			if ( first && ( rec == srvid) ) {
			    first = false;
				idIsLeader = true;
			}
			continue;  // a node in same zone, skip
		}

		if ( leader[zone].size() < 1 ) {
			leader[zone] = rec;
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
	//int numZones = getNumZones();
	int numFullZones = getNumFullZones();
	int seed = atoi( beacon.c_str() );

	uint len = nodeList_.size();
	uint dd = len/numFullZones;

	XXH64_hash_t hash;
	hash = XXH64( srvid.c_str(), srvid.size(), seed ) % len;
	int zoneid =  hash / dd;
	d("a22201 isLeader check srvid=[%s] zoneid=%d seed=%d len=%d numFullZones=%d dd=%d", s(srvid), zoneid, seed, len, numFullZones, dd );

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
				d("a11024 rec=[%s]===srvid=[%s] Leader=true, continue", s(rec), s(srvid) );
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
bool DynamicCircuit::getOtherLeadersAndThisFollowers( const sstr &beacon, const sstr &srvid, 
												  strvec &otherLeaders, strvec &followers )
{
	int numZones = getNumZones();
	int numFullZones = getNumFullZones();
	strvec leader(numZones);
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;

	uint len = nodeList_.size();
	uint dd = len/numFullZones;

	hash = XXH64( srvid.c_str(), srvid.size(), seed ) % len;
	int zoneid =  hash / dd;
	d("a40023 srvid=%s zoneid=%d", s(srvid), zoneid );

	bool idIsLeader = false;
	bool first = true;
	for ( unsigned int i=0; i < len; ++i ) {
		const sstr &rec  = nodeList_[i];
		hash = XXH64( rec.c_str(), rec.size(), seed ) % len;
		zone = hash / dd;

		if ( zoneid == zone ) {
			if ( first && ( srvid == rec ) ) {
				idIsLeader = true;
				first = false;
			}

			if ( srvid != rec  ) {
				followers.push_back( rec );
			}
		} else {
			// other zones
			if ( leader[zone].size() < 1 ) {
				leader[zone] = rec;
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


// L2
// Get leader of a follower
// return true if srvid is a leader
bool DynamicCircuit::getLeader( const sstr &beacon, const sstr &srvid, strvec &leader )
{
	int numFullZones = getNumFullZones();
	XXH64_hash_t hash;
	int seed = atoi( beacon.c_str() );
	int zone;

	uint len = nodeList_.size();
	uint dd = len/numFullZones;

	hash = XXH64( srvid.c_str(), srvid.size(), seed ) % len;
	int zoneid =  hash / dd;
	d("a40025 srvid=%s zoneid=%d", s(srvid), zoneid );

	bool idIsLeader = false;
	bool first = true;
	for ( unsigned int i=0; i < len; ++i ) {
		const sstr &rec  = nodeList_[i];
		hash = XXH64( rec.c_str(), rec.size(), seed ) % len;
		zone = hash / dd;

		if ( zoneid == zone ) {
			// same zone
			if ( first ) {
				if ( srvid == rec ) {
					idIsLeader = true;
				}
				first = false;
				leader.push_back( rec );
				break;  // leader
			}
		} 
	}

	return idIsLeader;
}
