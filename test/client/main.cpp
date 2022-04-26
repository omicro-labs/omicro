#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include "omicroclient.h"
#include "omicrotrxn.h"
#include "omicroquery.h"
#include "omicrokey.h"
#include "omlog.h"
#include "omjson.h"
#include "omtoken.h"
#include "ommsghdr.h"
INIT_LOGGING

/**************************************************************************************
**
**  This is example code for client to interact with Omicro
**
**	Usage: 
**	       omclient  key <userID>    (This will create keys of provided userID)
**
**         omclient  <serverIP>  <serverPort>  acct <userID>
**                                             Create account for provided userID
**
**         omclient  <serverIP>  <serverPort>  pay <fromUserID> <toUserID>  <amt>
**                                             Pay from <fromUserID> to <toUserID> <amt>
**
**         omclient  <serverIP>  <serverPort>  viewbal <userID>
**                                             View balance of user1 or user2
**
**         omclient  <serverIP>  <serverPort>  token <ownerID>
**                                             Create tokens under user <ownerID>. ownerID must exist already
**
**         omclient  <serverIP>  <serverPort>  viewtokens <ownerID>
**                                             View tokens under user <ownerID>.

**         omclient  <serverIP>  <serverPort>  viewtoken <ownerID> <tokenId>
**                                             View token under user <ownerID>.
**
**         omclient  <serverIP>  <serverPort>  xfer <fromUserID> <toUserID>  <amt> 
**                                             Transfer tokens from <fromUserID> to <toUserID> <amt>
**
**************************************************************************************/

void createUserKey( const std::string &uname);
void createAcct( const char *srv, int port, const std::string &username);
void createToken( const char *srv, int port, const std::string &owner );
void makePayment( const char *srv, int port, const std::string &from, const std::string &to, const std::string &amt );
void query( const std::string &qt, const char *srv, int port, const std::string &from, const std::string &token );
void readUserKey( std::string uname, std::string &secKey, std::string &pubKey );
void makeTransfer( const char *srv, int port, const std::string &from, const std::string &to, const std::string &amt );
std::string getHomeDir();

void help( const char *prog)
{
	printf("Usage: %s   key userID\n", prog);
	printf("Usage: %s  <serverIP>  <serverPort>  acct <userId>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  pay <fromId> <toId> <amount>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  viewbal <userId>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  token <ownerId>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  viewtokens <ownerId>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  viewtoken <ownerId> <tokenId>\n", prog );
	printf("Usage: %s  <serverIP>  <serverPort>  xfer <fromId> <toId> <amount>\n", prog );
}

int main(int argc, char* argv[])
{
	if ( argc < 3 ) {
		help(argv[0]);
		exit(1);
	}

	if ( 0 == strcmp(argv[1], "key" ) ) {
		if ( argc >= 3 ) {
			createUserKey( argv[2] );
			exit(0);
		} else {
			help(argv[0]);
			exit(1);
		}
	}

	//g_debug = true;

	const char *srv = argv[1];
	int port = atoi(argv[2]);

	if ( 0 == strcmp(argv[3], "acct" ) ) {
		if ( argc >= 5 ) {
			createAcct( srv, port, argv[4] );
		} else {
			help(argv[0]);
			exit(3);
		}
	} else if ( 0 == strcmp(argv[3], "token" ) ) {
		if ( argc >= 5 ) {
			createToken( srv, port, argv[4] );
			//                     onwerId
		} else {
			help(argv[0]);
			exit(3);
		}
	} else if ( 0 == strcmp(argv[3], "pay" ) ) {
		if ( argc >= 7 ) {
	   		makePayment( srv, port, argv[4], argv[5], argv[6] );
		} else {
			help(argv[0]);
			exit(5);
		}
	} else if ( 0 == strcmp(argv[3], "viewbal" ) ) {
		if ( argc >= 5 ) {
	    	query( "balance", srv, port, argv[4], "" );
		} else {
			help(argv[0]);
			exit(7);
		}
	} else if ( 0 == strcmp(argv[3], "viewtokens" ) ) {
		if ( argc >= 5 ) {
	    	query( "tokens", srv, port, argv[4], "" );
		} else {
			help(argv[0]);
			exit(7);
		}
	} else if ( 0 == strcmp(argv[3], "viewtoken" ) ) {
		if ( argc >= 6 ) {
	    	query( "token", srv, port, argv[4], argv[5] );
		} else {
			help(argv[0]);
			exit(7);
		}
	} else if ( 0 == strcmp(argv[3], "xfer" ) ) {
		if ( argc <= 7 ) {
	   		makeTransfer( srv, port, argv[4], argv[5], argv[6]  );
		} else {
			help(argv[0]);
			exit(5);
		}
	} else {
		help(argv[0]);
		exit(9);
	}

}

void createUserKey( const std::string &uname )
{
	std::string rt = getHomeDir();
	std::string f1 = rt + "/mysecretKey" + uname;
	std::string f2 = rt + "/mypublicKey" + uname;

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


void createAcct( const char *srv, int port, const std::string &userName)
{
	std::string secretKey, publicKey;
	readUserKey( userName, secretKey, publicKey );

	OmicroClient client( srv, port );
	if ( ! client.connectOK() ) {
		printf("Error connect=[%s:%d]\n", srv, port );
		return;
	}
	std::string nodePubkey = client.reqPublicKey( 3 );
	printf("clientproxy pubkey=[%s]\n", nodePubkey.c_str() );

	OmicroTrxn t;
	t.makeNewAcctTrxn(nodePubkey, secretKey, publicKey, userName );

	printf("a000234 client.sendTrxn() ...\n");
	std::string reply = client.sendTrxn( t );

	printf("%s confirm=[%s]\n", userName.c_str(), reply.c_str());
}

void createToken( const char *srv, int port, const std::string &userName )
{
	std::string secretKey, publicKey;
	readUserKey( userName, secretKey, publicKey );

	OmicroClient client( srv, port );
	if ( ! client.connectOK() ) {
		printf("Error connect=[%s:%d]\n", srv, port );
		return;
	}
	std::string nodePubkey = client.reqPublicKey( 3 );
	printf("clientproxy pubkey=[%s]\n", nodePubkey.c_str() );

	// mint some tokens: name and max are required, 
	// others are optional and can be added by creator in any way
	std::string token1 = "name: mytoken1, max: 10000";
	std::string nfttoken = "name: petnft, max: 1, url: http://abceruxv123";
	std::string token3 = "name: concert2023, max: 30000";
	std::vector<std::string> vec;
	vec.push_back(token1);
	vec.push_back(nfttoken);
	vec.push_back(token3);

	std::string tokensJson;
	OmToken::getMintJson(vec, tokensJson );
	if ( tokensJson.size() < 1 ) {
		printf("Error: tokensJson is empty\n");
		return;
	}

	OmicroTrxn t;
	t.makeNewTokenTrxn(nodePubkey, secretKey, publicKey, userName, tokensJson );

	printf("a000234 client.sendTrxn() ...\n");
	std::string reply = client.sendTrxn( t );

	printf("%s confirm=[%s]\n", userName.c_str(), reply.c_str());
}

void makePayment( const char * srv, int port, const std::string &from, const std::string &to, 
				  const std::string &amt )
{
	OmicroClient client( srv, port );
	if ( ! client.connectOK() ) {
		printf("Error connect=[%s:%d]\n", srv, port );
		return;
	}

	std::string nodePubkey = client.reqPublicKey( 3 );
	// printf("clientproxy nodepubkey=[%s]\n", nodePubkey.c_str() );

	std::string secretKey, publicKey;
	readUserKey( from, secretKey, publicKey );
	//OmicroUserKey::getUserId( publicKey, fromId );

	std::string secretKey2, publicKey2;
	readUserKey( to, secretKey2, publicKey2 );

	OmicroTrxn t;
	t.makeSimpleTrxn( nodePubkey, secretKey, publicKey, from, to, amt );
	// fake sender t.sender_ = "someotheruser";  // will fail

	std::string data; t.getTrxnData( data );
	printf("a2220 trxndata=[%s]\n", data.c_str() );

	std::string reply;
	printf("a000234 client.sendTrxn() ...\n");
	reply = client.sendTrxn( t );

	printf("%s confirm=[%s]\n", from.c_str(), reply.c_str());
}

void query( const std::string &qt, const char * srv, int port, const std::string &from, const std::string &token )
{
	OmicroClient client( srv, port );
	if ( ! client.connectOK() ) {
		printf("Error connect=[%s:%d]\n", srv, port );
		return;
	}
	std::string nodePubkey = client.reqPublicKey( 3 );
	// printf("clientproxy nodepubkey=[%s]\n", nodePubkey.c_str() );

	std::string secretKey, publicKey, fromId;
	readUserKey( from, secretKey, publicKey );
	printf("user from=[%s] pubkey=[%s]\n", from.c_str(), publicKey.c_str() );

	OmicroTrxn t;
	if ( qt == "balance" ) {
		t.makeAcctQuery( nodePubkey, secretKey, publicKey, from );
	} else if ( qt == "tokens" ) {
		t.makeTokensQuery( nodePubkey, secretKey, publicKey, from );
	} else if ( qt == "token" ) {
		t.makeOneTokenQuery( nodePubkey, secretKey, publicKey, from, token );
	} else {
		printf("Error [%s] is not supported\n", qt.c_str() );
		return;
	}

	std::string data; t.getTrxnData( data );
	printf("a2220 trxndata=[%s]\n", data.c_str() );

	printf("a000235 client.sendQuery() ...\n");
	std::string reply = client.sendQuery( t );

	printf("%s confirm=[%s]\n", from.c_str(), reply.c_str());
}

void readUserKey( std::string uname, std::string &secKey, std::string &pubKey )
{
	std::string rt = getHomeDir();
	std::string f1 = rt + "/mysecretKey" + uname;
	std::string f2 = rt + "/mypublicKey" + uname;

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

void makeTransfer( const char * srv, int port, const std::string &from, const std::string &to, 
				   const std::string &amt )
{
	OmicroClient client( srv, port );
	if ( ! client.connectOK() ) {
		printf("Error connect=[%s:%d]\n", srv, port );
		return;
	}

	std::string nodePubkey = client.reqPublicKey( 3 );
	// printf("clientproxy nodepubkey=[%s]\n", nodePubkey.c_str() );

	std::string secretKey, publicKey;
	readUserKey( from, secretKey, publicKey );

	std::string secretKey2, publicKey2;
	readUserKey( to, secretKey2, publicKey2 );

	OmicroTrxn t;

	std::string fttoken = std::string("name: mytoken1, amount: ") + amt;
	std::string nfttoken = "name: petnft"; 
	std::vector<std::string> vec;
	vec.push_back(fttoken);
	vec.push_back(nfttoken);

	std::string tokensJson;
	OmToken::getXferJson(vec, tokensJson );
	if ( tokensJson.size() < 1 ) {
		printf("Error: tokensJson is empty\n");
		return;
	}

	t.makeTokenTransfer( nodePubkey, secretKey, publicKey, from, tokensJson, to, amt );
	// fake sender t.sender_ = "someotheruser";  // will fail

	std::string data; t.getTrxnData( data );
	printf("a2220 trxndata=[%s]\n", data.c_str() );

	std::string reply;
	printf("a00231 client.sendTrxn() ...\n");
	reply = client.sendTrxn( t );

	printf("%s confirm=[%s]\n", from.c_str(), reply.c_str());
}

std::string getHomeDir()
{
	std::string rt;
	char *p = getenv("HOME");
	if ( p ) {
		rt = std::string(p) + "/.omicro";
	} else {
		rt = std::string("/tmp/.omicro");
	}

	::mkdir(rt.c_str(), 0700 );
	return rt;
}
