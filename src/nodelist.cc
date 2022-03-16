#include <string.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include "nodelist.h"
NodeList::NodeList()
{
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
	const char *fpath = "../conf/nodelist.conf";
	FILE *fp = fopen(fpath, "r");
	if ( fp == NULL ) {
		std::cout << "E10020 error open nodelist file " << fpath << std::endl;
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
	print();
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
	int L2max = 25;
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
		printf("%s\n", list_[i].c_str() );
		fflush(stdout);
	}
}

const sstr & NodeList::operator[](unsigned int i) const
{
	return list_[i];
}

