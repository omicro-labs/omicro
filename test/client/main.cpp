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
	//d("a02029 OmicroClient done");
	sstr pubkey = client.reqPublicKey( 3 );
	d("clientproxy pubkey=[%s]", s(pubkey) );

	OmicroTrxn t;
	//t.setInitTrxn();
	t.makeDummyTrxn(pubkey);

	//d("a20020 t.cipher=[%s]", s(t.cipher) );
	//d("a20020 t.signature=[%s]", s(t.signature) );

	i("a000234 client.sendTrxn() ...");
	sstr reply = client.sendTrxn( t );

	i("confirmation=[%s]", s(reply));

}

