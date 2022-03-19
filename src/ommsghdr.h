#ifndef _om_msg_hdr_h_
#define _om_msg_hdr_h_

#include "omicrodef.h"

#define OMHDR_SZ       24
#define OM_HDR_LEN_SZ  16
#define OM_PLAIN       'T'
#define OM_COMPRESSED  'C'

// [16 length of data][compress/plain][][][][][][][]{data...}
class OmMsgHdr
{
  public:
	OmMsgHdr( const char *str, int len);
    void setLength(ulong sz);
	ulong getLength() const;
	void  setPlain();
	void  setCompressed();
	const char *s() const;

  protected:
  	char *buf_;
};

#endif
