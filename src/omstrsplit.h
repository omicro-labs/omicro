#ifndef _om_strsplit_h_
#define _om_strsplit_h_
#include "omicrodef.h"

class OmStrSplit
{
	public:   

		OmStrSplit();
		OmStrSplit(const sstr& str, char sep = ' ', bool ignoreregion=false );
		OmStrSplit(const char *str, char sep = ' ', bool ignoreregion=false );
		
		void init(const char *str, char sep=' ', bool ignoreregion=false );

		void destroy();
		~OmStrSplit();

	    const sstr& operator[](int i ) const;
	    sstr& operator[](int i );
		sstr& last() const;
		long length() const;
		long size() const;
		long slength() const;
		bool  exists(const sstr &token) const;
		bool  contains(const sstr &token, sstr &rec ) const;
		void	print() const;
		void	printStr() const;
		void  shift();
		void  back();
		const char *c_str() const;
		void  pointTo( const char* str );

	private:
		sstr *list_;
		long length_;
		char sep_;
		int  start_;
		sstr _NULL;
		const char* pdata_;
};

#endif

