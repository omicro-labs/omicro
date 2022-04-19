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
#include <string.h>
#include <stdio.h>
#include <string>
#include "nodelist.h"
#include "omutil.h"
#include "omlimits.h"

NodeList::NodeList( const char *fpath )
{
	if ( NULL == fpath ) {
		nodeFile_ = "../conf/nodelist.conf";
	} else {
		nodeFile_ = fpath;
	}
	readFile();
}

NodeList::~NodeList()
{
}

// nostlist.conf
// pubkey1|ip1|port1
// pubkey2|ip1|port2
// pubkey3|ip2|port1
// pubkey4|ip2|port3
void NodeList::readFile()
{
	const char *fpath = s(nodeFile_);
	FILE *fp = fopen(fpath, "r");
	if ( fp == NULL ) {
		i("E10020 error open nodelist file [%s]", fpath);
		return;
	}

	char line[1024];
	int len;
	while ( NULL != fgets(line, 1000, fp) ) {
		if ( '#' == line[0] ) {
			continue;
		}
		len = strlen(line);
		if ( len < 4 ) {
			continue;
		}

		if ( ! strchr(line, '|') ) {
			continue;
		}

		if (line[len-1] == '\n' ) {
			line[len-1] = '\0'; 
		}
		list_.push_back( line );
	}

	fclose( fp );
	d("a02828 nodelist:  size=%d", list_.size() );
	//print();

}

bool
NodeList::getData( const sstr &rec, 
				   sstr &id, sstr &ip, sstr &port )
{
	// pubkey
	const char *p = rec.c_str();
	const char *q = strchr(p, '|');
	if ( q == NULL ) return false;
	id = sstr(p, q-p);

	// ip
	++q;
	p = q;
	q = strchr(p, '|');
	if ( q == NULL ) return false;
	ip = sstr(p, q-p);

	// port
	++q;
	port = sstr(q);

	return true;
}

Byte NodeList::getLevel() const
{
	// todo
	//int L2max = OM_L2_MAX_NODES;
	int L2max = 10000;
	int N = list_.size();
	if ( N < L2max ) {
		return 2;
	} else {
		return 3;
	}
}

unsigned int NodeList::length() const
{
	return list_.size();
}

unsigned int NodeList::size() const
{
	return list_.size();
}

void NodeList::print()
{
	for ( unsigned int i = 0; i < list_.size(); ++i ) {
		printf("a22233 %s\n", list_[i].c_str() );
		fflush(stdout);
	}
}

const sstr & NodeList::operator[](unsigned int i) const
{
	return list_[i];
}

