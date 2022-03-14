#ifndef _omicro_nodelist_h_
#define _omicro_nodelist_h_

#include <vector>
#include "omicrodef.h"

class NodeList
{
  public:
  	NodeList();
  	~NodeList();

	bool getData( const sstr &rec, sstr &pubkey, sstr &ip, sstr &port);
	Byte getLayer() const;
	void print();
	std::vector<sstr> list_;
	int  length() const;

  protected:
  	void readFile();
};

#endif
