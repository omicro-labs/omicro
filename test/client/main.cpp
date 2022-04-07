#include <string.h>
#include <string>
#include <sys/stat.h>
#include "omicroclient.h"
#include "omicrotrxn.h"
#include "omicroquery.h"
#include "omicrokey.h"
#include "omlog.h"
INIT_LOGGING

/******************************************************************
**
**  This is example code for client to interact with Omicro
**
**	Usage: omclient  key    (This will create private and public keys
**         omclient  <serverIP>  <serverPort>  <acct/req>
**                                              acct:  to create an account
**                                              req:  to query account status
**
**         omclient  <serverIP>  <serverPort>  pay  <amt>
**
******************************************************************/

void createUserKey( const std::string &num);
void createAcct( const char *srv, int port, std::string num);
void makePayment( const char *srv, int port, const std::string &from, const std::string &to, const std::string &amt );
void query( const char *srv, int port, const std::string &from );
void readUserKey( std::string num, std::string &secKey, std::string &pubKey );

void help( const char *prog)
{
	printf("Usage: %s  key\n", prog);
	printf("Usage: %s  <serverIP>  <serverPort>  <acct/req>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  pay <amount>\n", prog );
}

int main(int argc, char* argv[])
{
	if ( argc < 2 ) {
		help(argv[0]);
		exit(1);
	}

	if ( 0 == strcmp(argv[1], "key" ) ) {
		createUserKey("user1");
		createUserKey("user2");
		return 0;
	}

	if ( argc < 4 ) {
		help(argv[0]);
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
		if ( argc < 5 ) {
			help(argv[0]);
			exit(1);
		} else {
	    	makePayment( srv, port, "user1", "user2", argv[4] );
		}
	} else if ( 0 == strcmp(argv[3], "req" ) ) {
	    query( srv, port, "user1" );
	} else {
		printf("Usage: %s  <serverIP>  <serverPort>  <key/acct/pay>\n", argv[0]);
		exit(1);
	}

}

void createUserKey( const std::string &num )
{
	char *home = getenv("HOME");
	std::string dir = std::string(home) + "/.omicro";
	::mkdir(dir.c_str(), 0700);
	std::string f1 = dir + "/mysecretKey" + num;
	std::string f2 = dir + "/mypublicKey" + num;

	std::string secretKey, publicKey;
	OmicroUserKey::createKeyPairDL5( secretKey, publicKey );
	FILE *fp = fopen( f1.c_str(), "w");
	fprintf(fp, "%s", secretKey.c_str());
	fclose(fp);

	fp = fopen(f2.c_str(), "w");
	fprintf(fp, "%s", publicKey.c_str());
	fclose(fp);

	printf("Keys are created in %s %s\n", f1.c_str(), f2.c_str() );

	// debug
	/***
	std::string msg = "hihihihihihiend";
	std::string snmsg;
	OmicroUserKey::sign( msg, secretKey, snmsg );
	bool rc = OmicroUserKey::verify( snmsg, publicKey );
	printf("a30223 original keys verify rc=%d\n", rc );

	std::string secretKey2, publicKey2;
	readUserKey( num, secretKey2, publicKey2 );

	OmicroUserKey::sign( msg, secretKey, snmsg );
	rc = OmicroUserKey::verify( snmsg, publicKey );
	printf("a30224 readkeys verify rc=%d", rc );
	***/
}


void createAcct( const char *srv, int port, std::string num)
{
	std::string secretKey, publicKey;
	readUserKey( num, secretKey, publicKey );

	OmicroClient client( srv, port );
	std::string nodePubkey = client.reqPublicKey( 3 );
	printf("clientproxy pubkey=[%s]\n", nodePubkey.c_str() );

	OmicroTrxn t;
	t.makeNewAcctTrxn(nodePubkey, secretKey, publicKey );

	printf("a000234 client.sendTrxn() ...\n");
	std::string reply = client.sendTrxn( t );

	printf("confirmation=[%s]\n", reply.c_str());
}

void makePayment( const char * srv, int port, const std::string &from, const std::string &to, const std::string &amt )
{
	OmicroClient client( srv, port );
	std::string nodePubkey = client.reqPublicKey( 3 );
	// printf("clientproxy nodepubkey=[%s]\n", nodePubkey.c_str() );

	std::string secretKey, publicKey, fromId;
	readUserKey( from, secretKey, publicKey );
	OmicroUserKey::getUserId( publicKey, fromId );

	std::string secretKey2, publicKey2, toId;
	readUserKey( to, secretKey2, publicKey2 );
	OmicroUserKey::getUserId( publicKey2, toId );

	OmicroTrxn t;
	t.makeSimpleTrxn( nodePubkey, secretKey, publicKey, fromId, toId, amt );

	std::string data; t.getTrxnData( data );
	printf("a2220 trxndata=[%s]\n", data.c_str() );

	printf("a000234 client.sendTrxn() ...\n");
	std::string reply = client.sendTrxn( t );

	printf("confirmation=[%s]\n", reply.c_str());
}

void query( const char * srv, int port, const std::string &from )
{
	OmicroClient client( srv, port );
	std::string nodePubkey = client.reqPublicKey( 3 );
	// printf("clientproxy nodepubkey=[%s]\n", nodePubkey.c_str() );

	std::string secretKey, publicKey, fromId;
	readUserKey( from, secretKey, publicKey );
	printf("user from=[%s] pubkey=[%s]\n", from.c_str(), publicKey.c_str() );
	OmicroUserKey::getUserId( publicKey, fromId );
	printf("user id=[%s]\n", fromId.c_str() );

	OmicroTrxn t;
	t.makeAcctQuery( nodePubkey, secretKey, publicKey, fromId );

	std::string data; t.getTrxnData( data );
	printf("a2220 trxndata=[%s]\n", data.c_str() );

	printf("a000235 client.sendQuery() ...\n");
	std::string reply = client.sendQuery( t );

	printf("confirmation=[%s]\n", reply.c_str());
}

void readUserKey( std::string num, std::string &secKey, std::string &pubKey )
{
	char *home = getenv("HOME");
	std::string dir = std::string(home) + "/.omicro";
	::mkdir(dir.c_str(), 0700);
	std::string f1 = dir + "/mysecretKey" + num;
	std::string f2 = dir + "/mypublicKey" + num;

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
