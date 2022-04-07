#ifndef _om_msg_hdr_h_
#define _om_msg_hdr_h_

#include "omicrodef.h"

#define OMHDR_SZ       24
#define OM_HDR_LEN_SZ  16

// compression?
#define OM_PLAIN       'T'
#define OM_COMPRESSED  'C'

// msg type: query or trxn 
#define OM_RQ          'Q'
#define OM_TXN         'T'  // OM_RX
#define OM_XNQ         'X'

// query type
#define  OM_RQ_PK    'P'

// [16 length of data][compress/plain][msgtype][querytype][][][][][]{data...}
class OmMsgHdr
{
  public:
	OmMsgHdr( const char *str, int len, bool init);
    void setLength(ulong sz);
	ulong getLength() const;
	void  setPlain();
	void  setCompressed();
	char  getCompression();

	void  setMsgType( char t );
	char  getMsgType();

	void  setQueryType( char t );
	char  setQueryType();

	const char *s() const;

  protected:
  	char *buf_;
};

#endif
