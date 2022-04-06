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
	fence_ = sp[2];
	pad1_ = sp[3];
	pad2_ = sp[4];
	pad3_ = sp[5];
}

OmAccount::~OmAccount()
{
}

void OmAccount::str( sstr &res)
{
	res = balance_ + "|" + pubkey_ + "|" + fence_ 
	      + "|" + pad1_
	      + "|" + pad2_
	      + "|" + pad3_
		  ;
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

ulong OmAccount::getFence()
{
	return atoll(fence_.c_str());
}

void OmAccount::incrementFence()
{
	ulong cnt = atoll(fence_.c_str());
	++cnt;
	fence_ = std::to_string(cnt);
}


