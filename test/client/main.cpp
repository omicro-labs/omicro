#include <string>
#include "omutil.h"
#include "omicroclient.h"
#include "omicrotrxn.h"
#include "trxnstate.h"
#include "omstrsplit.h"
#include "ommsghdr.h"
#include "omicroquery.h"

INIT_LOGGING

int main(int argc, char* argv[])
{
	g_debug = true;
	OmicroClient client( argv[1], atoi(argv[2]) );
	d("a02029 OmicroClient done");
	sstr pubkey = client.reqPublicKey( 3 );
	d("clientproxy pubkey=[%s]", s(pubkey) );
	//return 0;

	OmicroTrxn t1;
	t1.makeDummyTrxn(pubkey);
	d("a393939 t1.setInitTrxn"); 
	t1.setInitTrxn();

	i("a000234 client.sendTrxn() ...");
	sstr reply = client.sendTrxn( t1 );

	i("reply=[%s]", s(reply));

}

