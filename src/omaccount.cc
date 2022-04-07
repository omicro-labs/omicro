#include <math.h>
#include "omaccount.h"
#include "omstrsplit.h"

OmAccount::OmAccount()
{
}

OmAccount::OmAccount( const char *rec )
{
	int i = 0;
	OmStrSplit sp(rec, '|');
	balance_ = sp[i++];
	tokentype_ = sp[i++];
	pubkey_ = sp[i++];
	keytype_ = sp[i++];
	//fence_ = sp[i++];
	out_ = sp[i++];
	in_ = sp[i++];
	pad1_ = sp[i++];
	pad2_ = sp[i++];
	pad3_ = sp[i++];
}

OmAccount::~OmAccount()
{
}

void OmAccount::str( sstr &res)
{
	res = balance_ 
	      + "|" + tokentype_ 
	      + "|" + pubkey_ 
	      + "|" + keytype_ 
		  + "|" + out_ 
		  + "|" + in_ 
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
	return atoll(out_.c_str());
}

void OmAccount::incrementFence()
{
	ulong cnt = atoll(out_.c_str());
	++cnt;
	out_ = std::to_string(cnt);
}

void OmAccount::incrementIn()
{
	ulong cnt = atoll(in_.c_str());
	++cnt;
	in_ = std::to_string(cnt);
}

