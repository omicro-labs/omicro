#include <string>
#include "omutil.h"
#include "omicroclient.h"
#include "omicrotrxn.h"

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

	sstr reply = client.sendMessage( t1.str(), true );
	i("reply=[%s]", s(reply));
}

