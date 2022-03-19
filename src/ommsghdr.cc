#include <assert.h>
#include <iostream>
#include "ommsghdr.h"

OmMsgHdr::OmMsgHdr( const char *str, int len)
{
	assert( len == OMHDR_SZ );
	buf_ = (char*)str;
}

void OmMsgHdr::setLength(ulong sz)
{
	char d[OM_HDR_LEN_SZ+1];
	sprintf(d, "%16ld", sz );  // 16 is OM_HDR_LEN_SZ
	for ( int i=0; i <OM_HDR_LEN_SZ; ++i ) {
		buf_[i] = d[i];
	}
}

ulong OmMsgHdr::getLength() const
{
	char v = buf_[OM_HDR_LEN_SZ];
	buf_[OM_HDR_LEN_SZ] = '\0';
	ulong e = atol(buf_);
	buf_[OM_HDR_LEN_SZ] = v;
	std::cout << "a33400 getLength e=" << e << std::endl;
	return e;
}

void OmMsgHdr::setPlain()
{
	buf_[OM_HDR_LEN_SZ] = OM_PLAIN;
}

void OmMsgHdr::setCompressed()
{
	buf_[OM_HDR_LEN_SZ] = OM_COMPRESSED;
}

const char *OmMsgHdr::s() const
{
	return buf_;
}

