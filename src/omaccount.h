#ifndef _om_account_h_
#define _om_account_h_

#include "omicrodef.h"

#define OM_ACCT_USER      "U"
#define OM_ACCT_CONTRACT  "C"

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
  	sstr pad1_;
  	sstr pad2_;
  	sstr pad3_;
    
};

#endif
