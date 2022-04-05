#ifndef _om_account_h_
#define _om_account_h_

#include "omicrodef.h"

class OmAccount
{
  public:
    OmAccount();
    OmAccount( const char *rec );
    ~OmAccount();

	void str( sstr &res );
	double addBalance(double amt);
	double getBalance(); 

  	sstr balance_;
  	sstr pubkey_;
    
};

#endif
