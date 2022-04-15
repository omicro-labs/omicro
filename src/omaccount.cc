#include <math.h>
#include "omaccount.h"
#include "omstrsplit.h"
#include "omjson.h"
#include "omdom.h"

OmAccount::OmAccount()
{
}

OmAccount::OmAccount( const char *recjson )
{
	OmDom od( recjson );
	od.get("A", accttype_ );
	od.get("B", balance_ );
	od.get("T", tokentype_ );
	od.get("P", pubkey_ );
	od.get("K", keytype_ );
	od.get("O", out_ );
	od.get("I", in_ );
	od.get("N", tokens_ );

}

OmAccount::~OmAccount()
{
}

void OmAccount::json( sstr &res)
{
	OmJson json;
	json.add("A", accttype_ );
	json.add("B", balance_ );
	json.add("T", tokentype_ );
	json.add("P", pubkey_ );
	json.add("K", keytype_ );
	json.add("O", out_ );
	json.add("I", in_ );
	json.add("N", tokens_ );
	json.json( res );
}

double  OmAccount::addBalance( double amt )
{
	if ( fabs(amt) > 10000000000 ) {
		return -15.0;
	}

	double bal = atof(balance_.c_str());
	bal += amt;
	if ( bal < 0.0 ) {
		return -20.0;
	}
	char buf[48];
	sprintf(buf, "%.6g", bal);
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
