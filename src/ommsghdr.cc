#include <string.h>
#include "omutil.h"
#include "ommsghdr.h"

OmMsgHdr::OmMsgHdr( const char *str, int len, bool init)
{
	if ( len != OMHDR_SZ ) {
		printf("E302837 Dev Error, exit\n");
		exit(1);
	}

	buf_ = (char*)str;

	if ( init ) {
		memset(buf_, ' ', len);
	}
	buf_[len] = '\0';
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
	d("a33400 OmMsgHdr::getLength() hdrlen=%d", e);
	if ( e < 2 ) {
		abort();
	}
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

char OmMsgHdr::getCompression()
{
	return buf_[OM_HDR_LEN_SZ];
}

void OmMsgHdr::setMsgType( char t)
{
	buf_[OM_HDR_LEN_SZ+1] = t;
}

char OmMsgHdr::getMsgType()
{
	return buf_[OM_HDR_LEN_SZ+1];
}

void  OmMsgHdr::setQueryType( char t )
{
	buf_[OM_HDR_LEN_SZ+2] = t;
}

char  OmMsgHdr::setQueryType()
{
	return buf_[OM_HDR_LEN_SZ+2];
}


const char *OmMsgHdr::s() const
{
	return buf_;
}

