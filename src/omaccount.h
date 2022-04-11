#ifndef _om_account_h_
#define _om_account_h_

#include "omicrodef.h"

#define OM_ACCT_USER      "U"
#define OM_ACCT_TOKEN     "T"
#define OM_ACCT_CONTRACT  "C"
#define OM_ACCT_ESCROW    "E"

class OmAccount
{
  public:
    OmAccount();
    OmAccount( const char *rec );
    ~OmAccount();

	void json( sstr &res );
	double addBalance(double amt);
	double getBalance(); 
	ulong  getFence();
	void   incrementFence();
	void   incrementIn();

  	sstr accttype_;
  	sstr balance_;
	sstr tokentype_;
  	sstr pubkey_;
  	sstr keytype_;
  	sstr out_;
  	sstr in_;
  	sstr tokens_;
};

#endif
