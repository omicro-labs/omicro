#include <string>
#include "omutil.h"
#include "omicroclient.h"
#include "omicrotrxn.h"
#include "trxnstate.h"
#include "omstrsplit.h"
#include "ommsghdr.h"
#include "omicroquery.h"
#include "omicrokey.h"

/******************************************************************
**
**   This is example code for client to interact with Omicro
**
******************************************************************/
INIT_LOGGING

void createUserKey( const sstr &num);
void createAcct( const char *srv, int port, sstr num);
void makePayment( const char *srv, int port, const sstr &from, const sstr &to, const sstr &amt );
void readUserKey( sstr num, sstr &secKey, sstr &pubKey );

int main(int argc, char* argv[])
{
	g_debug = true;

	if ( argc < 2 ) {
		printf("Usage: %s  key\n", argv[0]);
		printf("Usage: %s  <serverIP>  <serverPort>  <acct/pay>\n", argv[0]);
		exit(1);
	}

	if ( 0 == strcmp(argv[1], "key" ) ) {
		createUserKey("user1");
		createUserKey("user2");
		return 0;
	}

	if ( argc < 4 ) {
		printf("Usage: %s  key\n", argv[0]);
		printf("Usage: %s  <serverIP>  <serverPort>  <acct/pay>\n", argv[0]);
		exit(1);
	}

	const char *srv = argv[1];
	int port = atoi(argv[2]);

	if ( 0 == strcmp(argv[3], "key" ) ) {
		createUserKey("user1");
		createUserKey("user2");
	} else if ( 0 == strcmp(argv[3], "acct" ) ) {
		createAcct( srv, port, "user1" );
		createAcct( srv, port, "user2" );
	} else if ( 0 == strcmp(argv[3], "pay" ) ) {
	    makePayment( srv, port, "user1", "user2", "99.23" );
	} else {
		printf("Usage: %s  <serverIP>  <serverPort>  <key/acct/pay>\n", argv[0]);
		exit(1);
	}

}

void createUserKey( const sstr &num )
{
	sstr f1 = sstr("/tmp/mysecretKey") + num;
	sstr f2 = sstr("/tmp/mypublicKey") + num;

	sstr secretKey, publicKey;
	OmicroUserKey::createKeyPair( secretKey, publicKey );
	FILE *fp = fopen( f1.c_str(), "w");
	fprintf(fp, "%s", secretKey.c_str());
	fclose(fp);

	fp = fopen(f2.c_str(), "w");
	fprintf(fp, "%s", publicKey.c_str());
	fclose(fp);

	printf("Keys are created in %s %s\n", f1.c_str(), f2.c_str() );

	// debug
	/***
	sstr msg = "hihihihihihiend";
	sstr snmsg;
	OmicroUserKey::sign( msg, secretKey, snmsg );
	bool rc = OmicroUserKey::verify( snmsg, publicKey );
	d("a30223 original keys verify rc=%d", rc );

	sstr secretKey2, publicKey2;
	readUserKey( num, secretKey2, publicKey2 );

	OmicroUserKey::sign( msg, secretKey, snmsg );
	rc = OmicroUserKey::verify( snmsg, publicKey );
	d("a30224 readkeys verify rc=%d", rc );
	***/
}


void createAcct( const char *srv, int port, sstr num)
{
	sstr secretKey, publicKey;
	readUserKey( num, secretKey, publicKey );

	OmicroClient client( srv, port );
	sstr nodePubkey = client.reqPublicKey( 3 );
	d("clientproxy pubkey=[%s]", s(nodePubkey) );

	OmicroTrxn t;
	t.makeNewAcctTrxn(nodePubkey, secretKey, publicKey );

	i("a000234 client.sendTrxn() ...");
	sstr reply = client.sendTrxn( t );

	i("confirmation=[%s]", s(reply));
}

void makePayment( const char * srv, int port, const sstr &from, const sstr &to, const sstr &amt )
{
	OmicroClient client( srv, port );
	sstr nodePubkey = client.reqPublicKey( 3 );
	d("clientproxy nodepubkey=[%s]", s(nodePubkey) );

	sstr secretKey, publicKey, fromId;
	readUserKey( from, secretKey, publicKey );
	OmicroUserKey::getUserId( publicKey, fromId );

	sstr secretKey2, publicKey2, toId;
	readUserKey( to, secretKey2, publicKey2 );
	OmicroUserKey::getUserId( publicKey2, toId );

	OmicroTrxn t;
	t.makeSimpleTrxn( nodePubkey, secretKey, publicKey, fromId, toId, amt );

	sstr data; t.getTrxnData( data );
	i("a2220 trxndata=[%s]", s(data) );

	i("a000234 client.sendTrxn() ...");
	sstr reply = client.sendTrxn( t );

	i("confirmation=[%s]", s(reply));
}

void readUserKey( sstr num, sstr &secKey, sstr &pubKey )
{
	sstr f1 = sstr("/tmp/mysecretKey") + num;
	sstr f2 = sstr("/tmp/mypublicKey") + num;
	char buf[20000];

	FILE *fp = fopen(f1.c_str(), "r");
	if ( ! fp ) {
		printf("Error open [%s]\n", f1.c_str() );
		exit(11);
	}
	memset(buf, 0, 20000);
	fgets(buf, 20000, fp );
	fclose(fp);
	secKey = buf;

	fp = fopen( f2.c_str(), "r");
	if ( ! fp ) {
		printf("Error open [%s]\n", f2.c_str() );
		exit(12);
	}
	memset(buf, 0, 20000);
	fgets(buf, 20000, fp );
	fclose(fp);
	pubKey = buf;

	printf("Keys are read from %s %s\n", f1.c_str(), f2.c_str() );
}
