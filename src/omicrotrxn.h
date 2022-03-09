#ifndef _omicro_trxn_h_
#define _omicro_trxn_h_

class OmicroTrxn
{
  public:
  	OmicroTrxn();
  	~OmicroTrxn();

	char header[6];
	char sender[503];
	char receiver[503];
	char amount[16];   // "123456789.123456"
	char signature[64];
	char beacon[8];
	char timestamp[16];  // "123456789012.124"
	char trxnType[2];
	char assetType[2];

  protected:

};

#endif
