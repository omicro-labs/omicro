#include "omaccount.h"
#include "omstrsplit.h"

OmAccount::OmAccount()
{
}

OmAccount::OmAccount( const char *rec )
{
	OmStrSplit sp(rec, '|');
	balance_ = sp[0];
	pubkey_ = sp[1];
}

OmAccount::~OmAccount()
{
}

void OmAccount::str( sstr &res)
{
	res = balance_ + "|" + pubkey_;
}

double  OmAccount::addBalance( double amt )
{
	if ( fabs(amt) > 1000000000 ) {
		return -15.0;
	}

	double bal = atof(balance_.c_str());
	bal += amt;
	if ( bal < 0.0 ) {
		return -20.0;
	}
	char buf[48];
	sprintf(buf, "%.6f", bal);
	balance_ = buf;
	return bal;
}

double OmAccount::getBalance()
{
	return atof(balance_.c_str());
}
