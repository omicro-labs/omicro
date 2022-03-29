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

	OmicroTrxn t1;
	t1.makeDummyTrxn();
	d("a393939 t1.setInitTrxn"); 
	t1.setInitTrxn();

	// submit trxn
	/***
	d("t2.len=%d %d", t1.size(), strlen(t1.str()) );
	sstr reply = client.sendMessage( OM_RX, t1.str(), true );
	i("reply=[%s]\n\n", s(reply));

	OmStrSplit sp(reply, '|');
	sstr trxnId = sp[2];

	//sleep(3);

	OmicroQuery q;
	q.setTrxnId( trxnId );

	//reply = client.sendMessage( OM_RQ, q.str(), true );
	***/

	i("a000234 client.sendTrxn() ...");
	sstr reply = client.sendTrxn( t1 );

	i("reply=[%s]", s(reply));

}

