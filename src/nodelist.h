#ifndef _omicro_nodelist_h_
#define _omicro_nodelist_h_

#include <vector>
#include "omicrodef.h"

class NodeList
{
  public:
  	NodeList( const char *fpath = NULL);
  	~NodeList();

	static bool getData( const sstr &rec, sstr &id, sstr &ip, sstr &port);
	Byte getLevel() const;
	void print();
	std::vector<sstr> list_;
	unsigned int  length() const;
	unsigned int  size() const;
	const sstr & operator[](unsigned int i) const;

  protected:
  	void readFile();
	sstr nodeFile_;
};

#endif
