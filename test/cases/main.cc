#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <tchdb.h>
#include <tcfdb.h>
#define  RAPIDJSON_HAS_STDSTRING 1
#include "omicrodef.h"
#include "dynamiccircuit.h"
#include "omutil.h"
#include "nodelist.h"
#include "saber.h"
#include "ombase85.h"
#include "omaes.h"
#include "omicrokey.h"
#include "omstrsplit.h"
#include "omlog.h"
#include "omquery.h"
#include "omjson.h"
#include "omdom.h"
#include "omtoken.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include "dilith.h"

using bcode = boost::system::error_code;
using namespace rapidjson;
INIT_LOGGING

void rcopy( const rapidjson::Value &dv, Writer<StringBuffer> &writer );
int modifyTokens( const sstr &reqJson, sstr &fromTokens, sstr &toTokens );
int checkTokens( const sstr &reqJson, const sstr &fromTokens, const sstr &toTokens );
void tcopy();
void t1();
void t2();
void t3();
void t4();
void t5();
void t6();
void t7();
void t8();
void t9();
void t10();
void t11();
void t12();
void t13();
void t14();
void t15();
void t16();
void t17();
void t18();
int t20();
int t21();
int t22();
void t23();
void t24();
void t25();
void t26();
void t27();
void t28();
void t29();
void t30();

const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
void getRandStr(unsigned char *buf, int len)
{
	for ( int i=0; i <len; ++i ) {
		buf[i] = alphabet[rand()%26];
	}
}
void getRandStr2(char *buf, int len)
{
	for ( int i=0; i <len; ++i ) {
		buf[i] = alphabet[rand()%26];
	}
}

int main( int argc, char *argv[] )
{
	g_debug = true;

	//t1();
	//t2();
	//t3();
	//t4();
	//t5();
	//t6();
	//t7();
	// t8();
	//t9();
	//t10();
	//t11();
	t12();
	//t13();
	t14();
	t15();
	t16();
	t17();
	t18();
	t20();
	t21();
	// t22();
	//t23();
	//t24();
	//t25();
	//t26();
	//t27();
	//t28();
	//t29();
	t30();

	return 0;
}

// nodelist test
void t1()
{
	g_debug = 1;
	
	NodeList nl("conf/nodelist1.conf");
	printf("nodelist1:\n");
	nl.print();
	printf("\n");

	sstr beacon = "12345678";
	DynamicCircuit cir(nl);

	strvec zoneLeaders;
	cir.getZoneLeaders(beacon, zoneLeaders);
	printf("zoneLeaders:\n");
	dpvec(zoneLeaders);
	printf("\n");
	/**
	1|127.0.0.1|42050
	8|127.0.0.1|42058
	2|127.0.0.1|42051
	3|127.0.0.1|42052
	**/

	// sstr id = "1|127.0.0.1|42050";
	bool rc;
	for ( const auto &id : zoneLeaders ) {
		strvec followers;
		rc = cir.isLeader( beacon, id, true, followers);
		printf("111 leader [%s]=%d followers:\n", s(id), rc );
		dpvec(followers);
		printf("\n");
		followers.clear();

		strvec otherLeaders;
		rc = cir.getOtherLeaders( beacon, id, otherLeaders);
		printf("222 leader [%s]=%d otherLeaders:\n", s(id), rc);
		dpvec(otherLeaders);
		printf("\n");
		otherLeaders.clear();

		rc = cir.getOtherLeadersAndThisFollowers( beacon, id, otherLeaders, followers);
		printf("333 leader [%s]=%d otherLeaders:\n", s(id), rc);
		dpvec(otherLeaders);
		printf("\n");

		printf("333 leader [%s]=%d followers:\n", s(id), rc);
		dpvec(followers);
		printf("\n");
	}
}

int g_cnt = 0;
boost::asio::io_context io_context_;
boost::asio::steady_timer st(io_context_);
void printit(const boost::system::error_code& /*e*/)
{
  std::cout << "printit(): Hello, world! g_cnt=" << g_cnt << std::endl;
  ++g_cnt;
  if ( g_cnt < 5 ) {
      st.expires_from_now(std::chrono::seconds(3));
	  st.async_wait(printit); // non-blocking
  		std::cout << "printit(): renewed timer cnt=" << g_cnt << std::endl;
  }
}

// asyn wait test
void t2()
{
    st.expires_from_now(std::chrono::seconds(3));
	st.async_wait(printit); // non-blocking

    std::cout << "after st.async_wait" << std::endl;
	// after 3-seconds printit() will be called
    std::cout << "io.run() ..." << std::endl;
	io_context_.run();
}

std::mutex g_m;
std::condition_variable g_cv;
bool g_ready = false;

void foo_thread(int a)
{
	while ( g_cnt < 100000 ) {
		++ g_cnt;
		sleep(1);
		if (  g_cnt> 10 ) {
			std::unique_lock<std::mutex> lk(g_m);
			g_ready = true;
			std::cout << "g_ready raised to true, notify all" << std::endl;
			g_cv.notify_all();
			break;
		}
	}
}

void cv_wait_timeout( std::unique_lock<std::mutex> &lck, std::condition_variable &cv,  int sec )
{
    //auto endTime = std::chrono::now() + std::chrono::seconds(1);
	auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(sec) ;
    while( ! g_ready )
    {
       auto res = cv.wait_until(lck, endTime);
       if (res == std::cv_status::timeout)
       {
            // Add Timeout logic here
 			std::cout << "cv wait timedut" << std::endl;
            break;
       }
    }
	std::cout << "cv cv_wait_timeout done" << std::endl;
}

// test of post task to io context with thread
void t3()
{
	std::thread thread(foo_thread, 10); //

	boost::asio::post(io_context_, [&thread]() {
		sleep(1);
		std::cout << "t3 start running posted func" << std::endl;
		// check condition
		{
			std::cout << "wait on g_ready" << std::endl;
			std::unique_lock<std::mutex> lk(g_m);
			//g_cv.wait(lk, []{return g_ready;});
			cv_wait_timeout( lk, g_cv, 2);
			std::cout << "got g_ready is true , continue" << std::endl;
		}
		thread.join();

    }); // post is async -- returns immediately
	std::cout << "posting func is done" << std::endl;
	io_context_.run();
}

boost::asio::steady_timer timer(io_context_);
// test lambda async wait
void t4()
{
	// not stable in practice, may crash
	int a = 0;

	std::function<void(const bcode&)> checkStC = [&](const bcode &bc) {
			if ( bc == boost::asio::error::operation_aborted ) {
				i("E40028 checkStC error operation_aborted");
				return;
			}

			if ( a > 4 ) {
				d("a333 run my jobs ...");
				d("a333 run my jobs is done");
			} else {
				d("a5549381 onRecvL check that server is NOT in ST_C, wait 100 millsec more..");
				timer.expires_from_now(std::chrono::milliseconds(100));
				timer.async_wait( checkStC );
				++a;
			}
	};

	bcode bc;
	checkStC(bc);
	io_context_.run();
}


class Printer
{
  public:
  Printer(boost::asio::io_context& io)
    : timer_(io, boost::asio::chrono::seconds(1)), count_(0)
  {
  }

  void start()
  {
      timer_.async_wait(boost::bind(&Printer::print, this, 0, "hi"));
  }

  ~Printer()
  {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void print( int a, sstr b )
  {
    std::cout << "a=" << a << " b=" << b << std::endl;
    if (count_ < 5)
    {
      std::cout << count_ << std::endl;
      ++count_;
	  ++a;
	  b = b + "A";

      timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(100));
      timer_.async_wait(boost::bind(&Printer::print, this, a, b ));
    } else {
		d("run my jobs ...");
		d("run my jobs done");
	}
  }

  private:
  boost::asio::steady_timer timer_;
  int count_;
};

// test async wait with boost bind
void t5()
{
	// works well
    Printer p(io_context_);
	p.start();
    io_context_.run();
}

// test creating 1 million directories and files
void t6()
{
	sstr is, js, d, f;
	sstr topd = "tmpdir";
	::mkdir( topd.c_str(), 0700 );
	for ( int i=0; i < 1024; ++i ) {
		is = std::to_string(i);
		d = topd + "/" + is;
		::mkdir( d.c_str(), 0700 );
		for ( int j=0; j < 1024; ++j ) {
			js = std::to_string(j);
			d = topd + "/" + is + "/" + js; 
			::mkdir( d.c_str(), 0700 );
		}
	}
	// took 11.96 seconds

}

// test open and write to 1 millon files
void t7()
{
	sstr is, js, d, f;
	sstr topd = "tmpdir";

	for ( int i=0; i < 1024; ++i ) {
		is = std::to_string(i);
		d = topd + "/" + is;
		for ( int j=0; j < 1024; ++j ) {
			js = std::to_string(j);
			d = topd + "/" + is + "/" + js; 
			f = d + "/myfile.dbm";
			FILE *fp = fopen(f.c_str(), "a");
			fprintf(fp, "%s\n", d.c_str() );
			fclose(fp);
		}
	}
	// took 34.3 seconds
}

#if 0
// test libdbh2
void t8()
{
	DBHashTable *node;
  	const char *filename="test.dbh";
	unsigned char ksize=64;

	node = dbh_new(filename, &ksize, 0);
	if ( ! node ) {
		node = dbh_new(filename, &ksize, DBH_CREATE);
	}

	int rsize = DBH_MAXIMUM_RECORD_SIZE(node);
	d("rsize=%d", rsize );


	unsigned char key[ksize];
	unsigned char value[ksize];
	char rec[2*ksize];

	for ( int i=0; i < 100000; ++i ) {
		getRandStr(key, ksize);
		getRandStr(value, ksize);

		memcpy(rec, key, ksize);
		memcpy(rec+ksize, value, ksize);

		dbh_set_key(node, key);
		dbh_set_recordsize(node, 2*ksize);
		dbh_set_data( node, rec, 2*ksize);
		dbh_update(node);
	}
	// DBH_RECORD_SIZE(node)

	dbh_close(node);
	// 833 writes/second
}
#endif

// test  tokey cabinet hash db
void t9()
{
	TCHDB *hdb = tchdbnew();

	tchdbopen(hdb, "casket.tch", HDBOWRITER | HDBOCREAT);

	int ksize = 512;
	char key[ksize+1];
	char value[ksize+1];
	key[ksize] = '\0';
	value[ksize] = '\0';
	bool rc;

	for ( int i=0; i < 100000; ++i ) {
		getRandStr2(key, ksize);
		getRandStr2(value, ksize);
		//tchdbput2(hdb, (char*)key, (char*)value );  OK
		rc = tchdbput(hdb, (char*)key, ksize, (char*)value, ksize );
		if ( ! rc ) {
			int ecode = tchdbecode(hdb);
			d("error code [%s]", tchdberrmsg(ecode) );
		}
	}

	// k=64 v=64 bytes 100K records:  0m0.256s  -lz -lbz2 
	// k=64 v=64 bytes 1000K records 1.5 seconds   -lz -lbz2 
	// k=v=512 bytes  1000K records: 12 seconds  data file: 997MB
	//getRandStr2(key, ksize);
	char *pval = tchdbget2(hdb, (char*)key);
	if ( pval ) {
		d("k=[%s] v=[%s]", key, pval );
		free(pval);
	} else {
		int ecode = tchdbecode(hdb);
		d("error code [%s]", tchdberrmsg(ecode) );
	}

	tchdbclose(hdb);
}

// fixed size record tc
void t10()
{
	TCFDB *fdb = tcfdbnew();

	tcfdbopen(fdb, "casket.tcf", FDBOWRITER | FDBOCREAT);

	int ksize = 512;
	char key[ksize+1];
	char value[ksize+1];
	key[ksize] = '\0';
	value[ksize] = '\0';

	for ( int i=0; i < 100; ++i ) {
		getRandStr2(key, ksize);
		getRandStr2(value, ksize);
		if ( ! tcfdbput2(fdb, (void*)key, ksize, (void*)value, ksize ) ) {
			int ecode = tcfdbecode(fdb);
			d("put [%s] [%s] error code [%s]", key, value, tcfdberrmsg(ecode) );
			// fails????????
		}
	}

	// k=64 v=64 bytes 100K records:  0m0.256s  -lz -lbz2 
	// k=64 v=64 bytes 1000K records 1.5 seconds   -lz -lbz2 
	// k=v=512 bytes  1000K records: 12 seconds  data file: 997MB
	//getRandStr2(key, ksize);
	//char *pval = tcfdbget3(fdb, (char*)key);
	int valsz;
	char *pval = (char*)tcfdbget2(fdb, key, ksize, &valsz );
	if ( pval ) {
		d("k=[%s] v=[%s] vsize=%d", key, pval, valsz );
		free(pval);
	} else {
		int ecode = tcfdbecode(fdb);
		d("get error code [%s]", tcfdberrmsg(ecode) );
	}

	tcfdbclose(fdb);
}

// test boost uuid
void t11()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid uuidId = gen();
	sstr randomUUID = boost::lexical_cast<std::string>(uuidId);
	//uint64_t  uu = boost::lexical_cast<uint64_t>(uuidId);
	//d("uuid str=[%s] dd=[%ld]", s(randomUUID), uu );

	std::mt19937_64   engine(std::random_device{}());
	std::uniform_int_distribution<uint64_t> distribution;
	auto ui64 = distribution(engine);
	d("random ui64=%x", ui64);
	unsigned long id = gethostid();
	d("hostid=%x", id);
	auto ui64_2 = distribution(engine);
	d("random ui64_2=%x", ui64_2);
}

// SABER KEM
void t12()
{
  	uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  	uint8_t sk[CRYPTO_SECRETKEYBYTES];
  	uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
  	uint8_t ss_a[CRYPTO_BYTES], ss_b[CRYPTO_BYTES];

	unsigned char entropy_input[48];
	for (int i=0; i<48; i++) {
        entropy_input[i] = i;
	}
	randombytes_init(entropy_input, NULL, 256);

    //Generation of secret key sk, and public key pk pair
    crypto_kem_keypair(pk, sk);
	printf("pubk:\n");
	for ( int i=0; i < CRYPTO_PUBLICKEYBYTES; ++i ) {
		printf("%u", pk[i]);
	}
	printf("\n");
	printf("secretk:\n");
	for ( int i=0; i < CRYPTO_SECRETKEYBYTES; ++i ) {
		printf("%u", sk[i]);
	}
	printf("\n");

    //Key-Encapsulation call; input: pk; output: ciphertext ct, shared-secret ss_a;
    crypto_kem_enc(ct, ss_a, pk);
	printf("ciphertext:\n");
	for ( int i=0; i < CRYPTO_CIPHERTEXTBYTES; ++i ) {
		printf("%u", ct[i]);
	}
	printf("\n");
	printf("shared secret a:\n");
	for ( int i=0; i < CRYPTO_BYTES; ++i ) {
		printf("%u", ss_a[i]);
	}
	printf("\n");

    //Key-Decapsulation call; input: sk, ct; output: shared-secret ss_b;
    crypto_kem_dec(ss_b, ct, sk);


    // Functional verification: check if ss_a == ss_b?
    for(int i=0; i<SABER_KEYBYTES; i++)
    {
     	printf("%u \t %u\n", ss_a[i], ss_b[i]);
       	if(ss_a[i] != ss_b[i])
       	{
           	printf(" ----- ERR CCA KEM ------\n");
           	break;
       	}
    }
}

// test base85
void t13()
{
	const char *istr = "heolloeworkfjfjdkjfjend";
	size_t isz = strlen(istr);

	int32_t olen = ascii85_get_max_encoded_length(isz);
	if ( olen < 0 ) {
		printf("error 1\n");
	}

	char  obuf[olen];
	olen = encode_ascii85((uint8_t *)istr, isz, (uint8_t*)obuf, olen);

	int osz = strlen(obuf);
	int32_t olen2 = ascii85_get_max_decoded_length(osz);
    char  dbuf[olen2];
    int32_t ilen = decode_ascii85((uint8_t *)obuf, osz, (uint8_t*)dbuf, olen2);
	if (ilen < 0) {
		// error
		printf("error 3\n");
	}

	printf("isz=%ld ilen=%d\n", isz, ilen);
	printf("istr=[%s]\n", istr );
	printf("dbufg=[%s]\n", dbuf );
}

// base85 using ombase85 util
void t14()
{
	// original data
	int len=30;
	unsigned char in[len];
	printf("t14 input len=%d\n", len);
	for ( int i=0; i < len; ++i ) {
		in[i] = 5*i;
		printf("input i=%d  uint8_t=%d\n", i, in[i] );
	}
	printf("\n");

	sstr enc;
	base85Encode( in, len, enc );
	printf("enc=[%s]\n", s(enc) );

	unsigned char orig[len];
	int dd = base85Decode( enc, orig, len);
	printf("base85Decode dd=%d\n", dd );
	for ( int i=0; i < len; ++i ) {
		printf("output i=%d  uint8_t[i]=%d\n", i, orig[i] );
		if (  orig[i] != in[i] ) {
			printf("Error\n");
		}
	}
	printf("\n");
	printf("\n");

}


void t15()
{
    OmicroNodeKey k;
    sstr secretKey, publicKey;
    k.createKeyPairSB3( secretKey, publicKey );
    printf("t15 OmicroNodeKey secretKey=[%s]\n", secretKey.c_str() );
    printf("publicKey=[%s]\n", publicKey.c_str() );

    sstr msg = "do ntru keys encrypt and decrypt this message using post-quantum keys";
    sstr cipher, passwd, encMsg;
    k.encryptSB3( msg, publicKey, cipher, passwd, encMsg );

    printf("cipher=[%s]\n", cipher.c_str() );
    printf("passwd=[%s]\n", passwd.c_str() );
    printf("encMsg=[%s]\n", encMsg.c_str() );


    sstr plain;
    k.decryptSB3( encMsg, secretKey, cipher, plain );

    if ( plain == msg ) {
        printf("enc dec OK.  plain[%s] == msg[%s]\n", plain.c_str(), msg.c_str() );
    } else {
        printf("error\n");
    }

    printf("done\n");
	printf("\n");
}



// aes
void t16()
{
	sstr orig = "hihihihihiihihowr rururururruru warend toay|||||||||||999";
	sstr passwd = "1234567890123456";
	sstr cipher;

	aesEncrypt( orig, passwd, cipher );
	printf("t16 encs.size=%ld\n", cipher.size() );

	sstr plain;
	aesDecrypt( cipher, passwd, plain );
	printf("decs=[%s] len=%ld\n", plain.c_str(), plain.size() );

	sstr cipher2 = cipher.c_str();
	aesDecrypt( cipher2, passwd, plain );
	printf("2 decc=[%s] len=%ld\n", plain.c_str(), plain.size() );
    if ( plain == orig ) {
        printf("plain == orig OK\n");
    }
	printf("\n");


}

void t17()
{
	const char *s = "hi";
	sstr s1(s, 10);
	sstr s2(s, 20);

	sstr s3 = s1 + "|" + s2;
	printf("t17 s1=[%s] s2=[%s] s3=[%s]\n", s1.c_str(), s2.c_str(), s3.c_str() );
	printf("s1.size=[%lu] s2.size=[%lu] s3.size=[%lu]\n", s1.size(), s2.size(), s3.size() );

	OmicroNodeKey k;
	sstr sk, pk;

	k.createKeyPairSB3( sk, pk );

	//sstr msg = "hihihihihi world ÿ78f8 wwwwwwwwwwwwwwwwwwend||||||||||||999ÿ809`|||||||||0000";
	sstr msg = "18322328028669365593";
	sstr encMsg;
	sstr cipher, passwd;
	k.encryptSB3(msg, pk, cipher, passwd, encMsg );
	printf("encrypted =[%s] size=%lu strlen=%lu\n", encMsg.c_str(), encMsg.size(), strlen(encMsg.c_str()) );

	sstr plain;
	k.decryptSB3( encMsg, sk, cipher, plain );
	printf("decrypted =[%s] size=%lu strlen=%lu\n", plain.c_str(), plain.size(), strlen(plain.c_str()) );
    if ( plain == msg ) {
        printf("plain == msg, crypt/decrypt OK\n");
    }

	sstr signcipher, signature;
	k.signSB3( msg, pk, signcipher, signature );
	printf("signature =[%s] size=%lu strlen=%lu\n", signature.c_str(), signature.size(), strlen(signature.c_str()) );
	printf("signcipher=[%s] size=%lu strlen=%lu\n", signcipher.c_str(), signcipher.size(), strlen(signcipher.c_str()) );

	bool rc = k.verifySB3( msg, signature, signcipher, sk );
	printf("verify =[%d]\n", rc );
	printf("\n");

	//rc = k.verifySB3( msg, "aaaaaa", signcipher, sk );
	//printf("verify =[%d]\n", rc );

}

void t18()
{
	sstr s = "a1||a3|a4|||end";
	OmStrSplit sp(s, '|');
	printf("t18 splen=%ld\n", sp.size() );
	sp.print();
	printf("\n");
}


int t20()
{
  size_t i, j;
  int ret;
  size_t MLEN = 3259;
  size_t mlen, smlen;
  //uint8_t b;
  uint8_t m[MLEN + CRYPTO_BYTES_DL];
  uint8_t m2[MLEN + CRYPTO_BYTES_DL];
  uint8_t sm[MLEN + CRYPTO_BYTES_DL];
  uint8_t pk[CRYPTO_PUBLICKEYBYTES_DL];
  uint8_t sk[CRYPTO_SECRETKEYBYTES_DL];

  printf("t20\n");

  size_t NTESTS = 1;
  for(i = 0; i < NTESTS; ++i) {
    //randombytes_om(m, MLEN);
    //memset(m, '?', MLEN);

    crypto_sign_keypair(pk, sk);

    memset(m, '?', MLEN);

    crypto_sign(sm, &smlen, m, MLEN, sk);
	/**
	printf("signed msg: smlen=%d\n", smlen );
	for ( int j=0; j < smlen; ++j ) {
		printf("%c ", sm[j]);
	}
	printf("\n", sm[j]);
	**/

    ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);

    if(ret) {
      fprintf(stderr, "Verification failed\n");
      return -1;
    }

    if(smlen != MLEN + CRYPTO_BYTES_DL) {
      fprintf(stderr, "Signed message lengths wrong smlen=%ld MLEN + CRYPTO_BYTES_DL=%ld\n", smlen, MLEN + CRYPTO_BYTES_DL);
      return -1;
    }

    fprintf(stderr, "Signed message lengths MLEN=%ld smlen=%ld MLEN + CRYPTO_BYTES_DL=%ld CRYPTO_BYTES_DL=%d \n", 
			MLEN, smlen, MLEN + CRYPTO_BYTES_DL, CRYPTO_BYTES_DL);

    if(mlen != MLEN) {
      fprintf(stderr, "Message lengths wrong mlen=%ld MLEN=%ld\n", mlen, MLEN);
      return -1;
    }

    for(j = 0; j < MLEN; ++j) {
      if(m2[j] != m[j]) {
        fprintf(stderr, "Messages don't match\n");
        return -1;
      }
    }

	/***
    randombytes_om((uint8_t *)&j, sizeof(j));
    do {
      randombytes_om(&b, 1);
    } while(!b);
    sm[j % (MLEN + CRYPTO_BYTES_DL)] += b;
    ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);
    if(!ret) {
      fprintf(stderr, "Trivial forgeries possible\n");
      return -1;
    }
	***/
  }

  printf("CRYPTO_PUBLICKEYBYTES_DL = %d\n", CRYPTO_PUBLICKEYBYTES_DL);
  printf("CRYPTO_SECRETKEYBYTES_DL = %d\n", CRYPTO_SECRETKEYBYTES_DL);
  printf("CRYPTO_BYTES_DL = %d\n", CRYPTO_BYTES_DL);
  printf("\n");

  return 0;
}

int t21()
{
	sstr sk, pk;
	OmicroUserKey k;

    printf("t21 OmicroUserKey\n");

	k.createKeyPairDL5( sk, pk);
	//printf("sk=[%s]\n", s(sk));
	//printf("pk=[%s]\n", s(pk));

	sstr msg("hihihihihworldgogogoggoog-----------------------123--------------4321---------------end");

	sstr snmsg;
	k.signDL5( msg, sk, snmsg );
	printf("msg=[%s]\n",s(msg) );
	printf("snmsg=[%s]\n", s(snmsg) );

	bool rc = k.verifyDL5( snmsg, pk );
	printf("verify rc=%d   1 is OK\n\n", rc );

	rc = k.verifyDL5( "393933939393", pk );
	printf("verify rc=%d  0 is OK\n", rc );

    return 0;
}

int t22()
{
	tcopy();

	const char *injson = "{\"xed\": 123, \"bv\": true, \"vf\": 123.32, \"a\": \"123\", \"b\": \"xxx\", \"c\": [ {\"x1\": \"222\", \"cc2\": \"ccc\"}, {\"x2\": \"232\", \"cg2\": \"cbc\", \"arr\": [1, 2, 3, {\"inarr\": \"234\", \"ninin\": [3,45], \"xxxcc\": [\"aa\", \"cc\"] }]}] }"; 

	sstr outJson;
	std::vector<sstr> vec;
	vec.push_back("cc2");
	vec.push_back("x1");
	vec.push_back("xed");
	OmJson::stripJson(injson, vec, outJson);
	printf("\noutjson=%s\n", outJson.c_str() );

	sstr val;
	OmDom mydom( injson );
	mydom.get( "a", val );
	printf("a v=[%s]\n", val.c_str() );

	mydom.get( "vf", val );
	printf("vf v=[%s]\n", val.c_str() );

	mydom.get( "a123", val );
	printf("a123 v=[%s]\n", val.c_str() );

	return 0;
}

// dv is IsObject()
void rcopy( const rapidjson::Value &dv, Writer<StringBuffer> &writer )
{
	writer.StartObject();
	for ( auto& m : dv.GetObject() ) {
		// m.name.GetString(), m.value.GetType()

		writer.Key(m.name.GetString());

		const rapidjson::Value &v = m.value;
		if ( v.IsObject() ) {
			rcopy( v, writer);
		} else if ( v.IsArray() ) {
			writer.StartArray();
    		for ( rapidjson::SizeType i = 0; i < v.Size(); i++) {
				const rapidjson::Value &rv = v[i];
				if ( rv.IsObject() ) {
					rcopy(rv, writer); 
				} else {
					if ( rv.IsString() ) {
						//writer.Key(rv.GetString());
						//writer.String(rv.GetString());
						writer.String("");
					}
				}
			}
			writer.EndArray();
		} else if ( v.IsString()) {
			// leaf
			writer.String("");
			//writer.String(v.GetString());
		} else {
			writer.String("");
		}

	}
	writer.EndObject();
}

void tcopy()
{
	const char *json = "{\"xed\": 123, \"a\": \"123\", \"b\": \"xxx\", \"c\": [ {\"x1\": \"222\", \"cc2\": \"ccc\"}, {\"x2\": \"232\", \"cg2\": \"cbc\", \"arr\": [1, 2, 3, {\"inarr\": \"234\", \"ninin\": [3,45], \"xxxcc\": [\"aa\", \"cc\"] }]}] }"; 
	printf("orig json=%s\n", json );
	rapidjson::Document dom1;
	dom1.Parse( json );
	if ( dom1.HasParseError() ) {
		printf("a9393 has parse error\n");
		exit(1);
	}

	StringBuffer sbuf;
	Writer<StringBuffer> writer(sbuf);

	assert( dom1.IsObject() );
	rcopy(dom1, writer);

	printf("a33448 sbuf=\n");
	printf("%s\n", sbuf.GetString() );

	rapidjson::Document dom2;
	dom2.Parse( sbuf.GetString() );
	if ( dom2.HasParseError() ) {
		printf("a9393 has parse error\n");
		exit(1);
	}

	//dom2.GetString();
	//printf("dom2  %s\n", dom2.GetString());
	sstr val;
	OmDom mydom( sbuf.GetString() );
	mydom.get( "a", val );
	printf("a v=[%s]\n", val.c_str() );

	mydom.get( "a123", val );
	printf("a123 v=[%s]\n", val.c_str() );

}

void t23()
{
	std::string token = " name: aaa, max: 123 , url: http://uurfjfppsvb, prop1: ffk, prop2: eieied";
	token = " name: aaa ,,  max: 123 , , url: http://uurfjfppsvb, prop1: ffk , prop2: eieied ,, 	  ";
	token = " name: aaa ,,  max: , , url: http://uurfjfppsvb, prop1: ffk , prop2: eieied ,, 	  ";
	token = " , , .:  ";
	i("token=[%s]", s(token) );

    std::string key, val;
    const char *p = token.c_str();
    while ( *p == ' ' || *p == '\t' ) ++p; // q at ,
    if ( *p == '\0' ) return;
    const char *q = p;
    while ( *p != '\0' ) {
        while ( *q != ':' && *q != '\0' ) ++q;
        if ( *q == '\0' ) {
			break;
		}
        key = std::string(p, q-p);
        ++q;  // pass ':'
        while ( *q == ' ' || *q == '\t' ) ++q;
        if ( *q == '\0' ) {
			break;
		}
        p = q;
        while ( *q != ' ' && *q != '\t' && *q != ',' && *q != '\0' ) ++q; // q at ,
        val = std::string(p, q-p);
        while ( *q == ' ' || *q == '\t' || *q == ',' ) ++q;
        if ( *q == '\0' ) break;
        while ( *q == ' ' || *q == '\t' || *q == ',' ) ++q; 
        p = q;
    }
}

void t24()
{
	using namespace rapidjson;

	// const char *json = "{\"xed\": 123, \"a\": \"123\", \"b\": \"xxx\", \"c\": [ {\"x1\": \"222\", \"cc2\": \"ccc\"}, {\"x2\": \"232\", \"cg2\": \"cbc\", \"arr\": [1, 2, 3, {\"inarr\": \"234\", \"ninin\": [3,45], \"xxxcc\": [\"aa\", \"cc\"] }]}] }"; 
	const char *json = "[{\"xed\": 123, \"a\": \"123\"}, {\"xed2\": 123, \"b\": \"143\"} ]"; 
	const char *json2 = "{\"xed\": 123, \"a\": \"123\"}, {\"xed2\": 123, \"b\": \"143\"}"; 

	printf("orig json=%s\n", json );
	Document dom1;
	dom1.Parse( json );
	if ( dom1.HasParseError() ) {
		printf("a9393 has parse error\n");
		exit(1);
	}

	Document dom2;
	dom1.Parse( json2 );
	if ( dom2.HasParseError() ) {
		printf("a1393 has parse error\n");
		exit(1);
	}

	/***
	std::string s;
	dom1.StartObject();
		s = "newk1";
	    dom1.Key( s.c_str(), s.size(), true );

		s = "v1";
	    dom1.String( s.c_str(), s.size(), true );

	dom1.EndObject(1);
	***/
	// dom1.AddMember("aaa", dom2, dom1.GetAllocator() );

	StringBuffer sbuf;
    Writer<StringBuffer> writer(sbuf);

	//writer.StartArray();
	//writer.StartObject();
		writer.Key("aaa1");
		writer.String("aaa1");
	//writer.EndObject();
	//writer.EndArray();

    dom1.Accept(writer);
	printf( "dom1: [%s]\n", sbuf.GetString() );
}

void t25()
{
	const char *json1 = "[{\"xed\": 123, \"a\" : \"123\"}, {\"xed2\": 123, \"b\": \"143\"} ]"; 
	const char *json2 = "{\"xed\": 123, \"a\": \"123\"}, {\"xed2\": 123, \"b\": \"143\"}"; 
	const char *json3 = "{\"xd\": 123, \"f\" : \"123\"}, {\"xed4\": 123, \"bh\": \"143\"}"; 
	
	bool rc = OmToken::hasDupNames( json1, json2 );
	printf(" json1--json2 hasDupNames rc=%d\n\n", rc );

	rc = OmToken::hasDupNames( json1, json3 );
	printf(" json1--json3 hasDupNames rc=%d\n", rc );

}

void t26()
{
	Document dom0;
	dom0.Parse( "[]" );
	if ( dom0.HasParseError() ) {
		printf("a2393 has parse error on empty string\n");
		exit(1);
	}

	const std::string json1 = "[{\"name\": \"tok1\", \"bal\" : \"123\"}, {\"name\": \"tok2\", \"bal\": \"143\"} ]"; 
	Document dom1;
	dom1.Parse( json1 );
	if ( dom1.HasParseError() ) {
		printf("a9393 has parse error\n");
		exit(1);
	}

	const std::string change = "[{\"name\": \"tok1\", \"amount\" : \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"} ]"; 
	Document chdom;
	chdom.Parse( change );
	if ( chdom.HasParseError() ) {
		printf("a9393 has parse error\n");
		exit(1);
	}

	assert( chdom.IsArray() );
	Value::ConstMemberIterator itr;
	double damt;
	char buf[32];

   	for ( rapidjson::SizeType i = 0; i < chdom.Size(); i++) {
		//const rapidjson::Value &rv = chdom[i];
		rapidjson::Value &rv = chdom[i];
		if ( ! rv.IsObject() ) {
			continue;
		}

		// rv is object { "name": "tok1", "amount": "234" }
		itr = rv.FindMember("name");
		if ( itr == rv.MemberEnd() ) {
			continue;
		}
		const sstr &name = itr->value.GetString();

		itr = rv.FindMember("amount");
		if ( itr == rv.MemberEnd() ) {
			continue;
		}
		const sstr &amt = itr->value.GetString();
		printf("name=[%s] amt=[%s]\n", s(name), s(amt) );

		//rv["amount"] = rapidjson::Value("100022");
		rv["amount"].SetString("100022");
	}

   	for ( rapidjson::SizeType i = 0; i < chdom.Size(); i++) {
		const rapidjson::Value &rv = chdom[i];
		if ( ! rv.IsObject() ) {
			continue;
		}

		// rv is object { "name": "tok1", "amount": "234" }
		itr = rv.FindMember("name");
		if ( itr == rv.MemberEnd() ) {
			continue;
		}
		const sstr &name = itr->value.GetString();
		if ( name.size() > 128 ) {
			continue;
		}

		itr = rv.FindMember("amount");
		if ( itr == rv.MemberEnd() ) {
			continue;
		}
		const sstr &amt = itr->value.GetString();
		printf("name=[%s] amt=[%s]\n", s(name), s(amt) );
		if ( amt.size() > 30 ) {
			continue;
		}

   		for ( rapidjson::SizeType j = 0; j < dom1.Size(); j++) {
			rapidjson::Value &v = dom1[j];
			if ( ! v.IsObject() ) {
				continue;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &tname = itr->value.GetString();
			if ( tname != name ) {
				continue;
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &bal = itr->value.GetString();
			damt = atof(bal.c_str()) - atof(amt.c_str());
			if ( damt <= 0.0 ) {
				damt = 0;
			}

			sprintf(buf, "%.6f", damt );
			v["bal"].SetString( rapidjson::StringRef(buf) );
			//v["bal"].SetString( "999.12" );
		}
	}

   	for ( rapidjson::SizeType j = 0; j < dom1.Size(); j++) {
			const rapidjson::Value &v = dom1[j];
			if ( ! v.IsObject() ) {
				continue;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &tname = itr->value.GetString();

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &bal = itr->value.GetString();
			printf("tname=[%s] bal=[%s]\n", s(tname), s(bal) );
	}

}

void t27()
{
	sstr fromjson = "[{\"name\": \"tok1\", \"bal\" : \"123\"}, {\"name\": \"tok2\", \"bal\": \"147\"} ]"; 
	sstr tojson = "[{\"name\": \"tok5\", \"bal\" : \"80\"}, {\"name\": \"tok9\", \"bal\": \"90\"} ]"; 
	sstr reqjson = "[{\"name\": \"tok1\", \"amount\": \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"}]"; 
	//sstr reqjson = "{\"name\": \"tok1\", \"amount\": \"10\"}";

	printf("orig fromjson=%s\n", s(fromjson) );
	printf("orig tojson=%s\n", s(tojson) );
	printf("reqjson=%s\n", s(reqjson) );
	int rc = checkTokens( reqjson, fromjson, tojson);
	printf("check rc=%d\n\n", rc );
	rc = modifyTokens( reqjson, fromjson, tojson);
	printf("fromjson=%s\n", s(fromjson) );
	printf("tojson=%s\n", s(tojson) );
	printf("rc=%d\n\n", rc );

	fromjson = "[{\"name\": \"tok1\", \"bal\" : \"123\", \"yrl\": \"baba\"}, {\"name\": \"tok2\", \"bal\": \"143\"} ]"; 
	tojson = "[{\"name\": \"tok1\", \"bal\" : \"200\"}, {\"name\": \"tok9\", \"bal\": \"143\"} ]"; 
	reqjson = "[{\"name\": \"tok1\", \"amount\": \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"}]"; 
	printf("orig fromjson=%s\n", s(fromjson) );
	printf("orig tojson=%s\n", s(tojson) );
	printf("reqjson=%s\n", s(reqjson) );
	rc = checkTokens( reqjson, fromjson, tojson);
	printf("check rc=%d\n\n", rc );
	rc = modifyTokens( reqjson, fromjson, tojson);
	printf("fromjson=%s\n", s(fromjson) );
	printf("tojson=%s\n", s(tojson) );
	printf("rc=%d\n\n", rc );

	fromjson = "[{\"name\": \"tok1\", \"bal\" : \"123\", \"yrl\": \"baba\"}, {\"name\": \"tok2\", \"bal\": \"203\"} ]"; 
	tojson = "[{\"name\": \"tok1\", \"bal\" : \"200\"}, {\"name\": \"tok9\", \"bal\": \"143\"}, {\"name\": \"tok2\", \"bal\": \"3\"} ]"; 
	reqjson = "[{\"name\": \"tok1\", \"amount\": \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"}]"; 
	printf("orig fromjson=%s\n", s(fromjson) );
	printf("orig tojson=%s\n", s(tojson) );
	printf("reqjson=%s\n", s(reqjson) );
	rc = checkTokens( reqjson, fromjson, tojson);
	printf("check rc=%d\n\n", rc );
	rc = modifyTokens( reqjson, fromjson, tojson);
	printf("fromjson=%s\n", s(fromjson) );
	printf("tojson=%s\n", s(tojson) );
	printf("rc=%d\n\n", rc );

	fromjson = "[{\"name\": \"tok1\", \"bal\" : \"123\", \"yrl\": \"baba\"}, {\"name\": \"tok2\", \"bal\": \"203\"} ]"; 
	tojson = ""; 
	reqjson = "[{\"name\": \"tok1\", \"amount\": \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"}]"; 
	printf("orig fromjson=%s\n", s(fromjson) );
	printf("orig tojson=%s\n", s(tojson) );
	printf("reqjson=%s\n", s(reqjson) );
	rc = checkTokens( reqjson, fromjson, tojson);
	printf("check rc=%d\n\n", rc );
	rc = modifyTokens( reqjson, fromjson, tojson);
	printf("fromjson=%s\n", s(fromjson) );
	printf("tojson=%s\n", s(tojson) );
	printf("rc=%d\n\n", rc );

	fromjson = "[]"; 
	tojson = "[{\"name\": \"tok1\", \"bal\" : \"200\"}, {\"name\": \"tok9\", \"bal\": \"143\"}, {\"name\": \"tok2\", \"bal\": \"3\"} ]"; 
	reqjson = "[{\"name\": \"tok1\", \"amount\": \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"}]"; 
	printf("orig fromjson=%s\n", s(fromjson) );
	printf("orig tojson=%s\n", s(tojson) );
	printf("reqjson=%s\n", s(reqjson) );
	rc = checkTokens( reqjson, fromjson, tojson);
	printf("check rc=%d\n\n", rc );
	rc = modifyTokens( reqjson, fromjson, tojson);
	printf("fromjson=%s\n", s(fromjson) );
	printf("tojson=%s\n", s(tojson) );
	printf("rc=%d\n\n", rc );


	fromjson = "[{\"name\": \"tok1\", \"bal\" : \"123\", \"yrl\": \"baba\"}, {\"name\": \"tok3\", \"bal\": \"203\"} ]"; 
	tojson = "[{\"name\": \"tok1\", \"bal\" : \"200\"}, {\"name\": \"tok9\", \"bal\": \"143\"}, {\"name\": \"tok2\", \"bal\": \"3\"} ]"; 
	reqjson = "[{\"name\": \"tok1\", \"amount\": \"10\"}, {\"name\": \"tok2\", \"amount\": \"20\"}]"; 
	printf("orig fromjson=%s\n", s(fromjson) );
	printf("orig tojson=%s\n", s(tojson) );
	printf("reqjson=%s\n", s(reqjson) );
	rc = checkTokens( reqjson, fromjson, tojson);
	printf("check rc=%d\n\n", rc );
	rc = modifyTokens( reqjson, fromjson, tojson);
	printf("fromjson=%s\n", s(fromjson) );
	printf("tojson=%s\n", s(tojson) );
	printf("rc=%d\n\n", rc );

}


int modifyTokens( const sstr &reqJson, sstr &fromTokens, sstr &toTokens )
{
	using namespace rapidjson;
	Document fromdom;
	fromdom.Parse( fromTokens );
	if ( fromdom.HasParseError() ) {
		i("E32231 modifyTokens fromTokens invalid");
		return -10;
	}

	Document todom;
	if ( toTokens.size() > 0 ) {
		todom.Parse( toTokens );
		if ( todom.HasParseError() ) {
			i("E32233 modifyTokens toTokens invalid");
			return -20;
		}
	} else {
		todom.Parse( "[]" );
	}

	Document chdom;
	chdom.Parse( reqJson );
	if ( chdom.HasParseError() ) {
		i("E32235 modifyTokens reqJson invalid");
		i("reqJson=%s", s(reqJson) );
		return -30;
	}

	if ( ! chdom.IsArray() ) {
		i("E33035 modifyTokens chdom is not array");
		return -40;
	}

	Value::ConstMemberIterator itr;
	double damt;
	char buf[32];
	sstr xferAmount;

	bool fromHasToken;
	bool toHasToken;

   	for ( rapidjson::SizeType idx = 0; idx < chdom.Size(); ++idx) {
		const rapidjson::Value &rv = chdom[idx];
		if ( ! rv.IsObject() ) {
			continue;
		}

		// rv is object { "name": "tok1", "amount": "234" }
		itr = rv.FindMember("name");
		if ( itr == rv.MemberEnd() ) {
			continue;
		}
		const sstr &xferName = itr->value.GetString();
		// bug above  name
		if ( xferName.size() > 128 ) {
			continue;
		}

		itr = rv.FindMember("amount");
		if ( itr == rv.MemberEnd() ) {
			xferAmount = "1";
			printf("a29292 no amount, use 1\n");
		} else {
    		if ( itr->value.IsString() ) {
    			//printf("[%s] amount a33933930 is string !!!!\n", s(name));
    			// bug
        		const sstr &amt = itr->value.GetString();
        		if ( amt.size() > 30 ) {
        			continue;
        		}
        		if ( atof( amt.c_str() ) <= 0 ) {
        			i("E37023 error amt=[%s]", s(amt) );
        			return -45;
        		}
        		xferAmount = amt;
    		} else {
        		i("E31043 error not string" );
        		return -46;
    		}
		}

		d("a2221 xferAmount=[%s]", s(xferAmount));

		// deduct from tokens
		fromHasToken = false;
   		for ( rapidjson::SizeType j = 0; j < fromdom.Size(); ++j) {
			rapidjson::Value &v = fromdom[j];
			if ( ! v.IsObject() ) {
				continue;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &tname = itr->value.GetString();
			if ( tname != xferName ) {
				continue;
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &bal = itr->value.GetString();

			damt = atof(bal.c_str()) - atof(xferAmount.c_str());
			if ( damt < 0.0 ) {
				i("E21722 error xferamount=[%s] iver bal=[%s]", xferAmount.c_str(), bal.c_str() );
				return -50;
			}

			sprintf(buf, "%.6f", damt );
			v["bal"].SetString( buf, fromdom.GetAllocator() );
			d("a22083 new from account xfername=[%s] bal_buf=[%s] origbal=[%s] xferAmount=[%s]", s(xferName), buf, s(bal), s(xferAmount) );
			fromHasToken = true;
			break;
		}

		if ( ! fromHasToken ) {
			//continue;  // do not touch todom
			i("E45023 fromacct has no such token [%s]", s(xferName) );
			return -55;
		}

		// check existing tokens in to account
		toHasToken = false;
   		for ( rapidjson::SizeType j = 0; j < todom.Size(); j++) {
			rapidjson::Value &v = todom[j];
			if ( ! v.IsObject() ) {
				continue;
			}

			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				continue;
			}

			const sstr &tname = itr->value.GetString();
			if ( tname != xferName ) {
				continue;
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}

			toHasToken = true;
			const sstr &bal = itr->value.GetString();
			damt = atof(bal.c_str()) + atof(xferAmount.c_str());
			sprintf(buf, "%.6f", damt );

			v["bal"].SetString( buf, todom.GetAllocator() );
			d("a222001 toaccount bal xfername=[%s] to=[%s] bal=[%s] xferAmoun=[%s]", s(xferName), buf, s(bal), s(xferAmount) );
			break;
		}

		d("a93938 toHasToken=%d", toHasToken );

		if ( ! toHasToken ) {
			// find the oject with the mathching name in from
   			for ( rapidjson::SizeType j = 0; j < fromdom.Size(); ++j) {
				const rapidjson::Value &v = fromdom[j];
				if ( ! v.IsObject() ) {
					continue;
				}
				itr = v.FindMember("name");
				if ( itr == v.MemberEnd() ) {
					continue;
				}
				const sstr &tname = itr->value.GetString();
				if ( tname != xferName ) {
					continue;
				}

				// v is the keys-values object
				Value obj(kObjectType);
				obj.SetObject();
				//todo qwer wrong call for todom
				int cnt = 0;
				for (auto& kv : v.GetObject()) {
					if ( 0 == strcmp(kv.name.GetString(), "bal" ) ) {
						obj.AddMember( StringRef(kv.name.GetString()), xferAmount, todom.GetAllocator() );
					} else {
						obj.AddMember( StringRef(kv.name.GetString()), StringRef(kv.value.GetString()), todom.GetAllocator() );
					}
					++cnt;
				}
				if ( cnt > 0 ) {
					todom.PushBack( obj, todom.GetAllocator() ); 
				}
				break;
			}
		}

		d("a22222\n");
	}

	StringBuffer sbuf;
	Writer<StringBuffer> writer(sbuf);
	fromdom.Accept(writer);
	fromTokens = sbuf.GetString();

	StringBuffer sbuf2;
	Writer<StringBuffer> writer2(sbuf2);
	todom.Accept(writer2);
	toTokens = sbuf2.GetString();

	//d("a23017 fromTokens=[%s]", s(fromTokens) );
	//d("a23017 toTokens=[%s]", s(toTokens) );

	return 0;
}

int checkTokens( const sstr &reqJson, const sstr &fromTokens, const sstr &toTokens )
{
	using namespace rapidjson;
	Document fromdom;
	fromdom.Parse( fromTokens );
	if ( fromdom.HasParseError() ) {
		i("E32231 modifyTokens fromTokens invalid");
		return -10;
	}

	Document todom;
	if ( toTokens.size() > 0 ) {
		todom.Parse( toTokens );
		if ( todom.HasParseError() ) {
			i("E32233 modifyTokens toTokens invalid");
			return -20;
		}
	} else {
		todom.Parse( "[]" );
	}

	Document chdom;
	chdom.Parse( reqJson );
	if ( chdom.HasParseError() ) {
		i("E32235 modifyTokens reqJson invalid");
		i("reqJson=%s", s(reqJson) );
		return -30;
	}

	if ( ! chdom.IsArray() ) {
		i("E33035 modifyTokens chdom is not array");
		return -40;
	}

	Value::ConstMemberIterator itr;
	sstr xferAmount;
	bool fromHasToken;

	// check each tolen to be transfered
   	for ( rapidjson::SizeType idx = 0; idx < chdom.Size(); ++idx) {
		const rapidjson::Value &rv = chdom[idx];
		if ( ! rv.IsObject() ) {
			continue;
		}

		// rv is object { "name": "tok1", "amount": "234" }
		itr = rv.FindMember("name");
		if ( itr == rv.MemberEnd() ) {
			continue;
		}
		const sstr &xferName = itr->value.GetString();
		// bug above  name
		if ( xferName.size() > 128 ) {
			continue;
		}

		itr = rv.FindMember("amount");
		if ( itr == rv.MemberEnd() ) {
			xferAmount = "1";
			printf("a29292 no amount, use 1\n");
		} else {
    		if ( itr->value.IsString() ) {
    			//printf("[%s] amount a33933930 is string !!!!\n", s(name));
    			// bug
        		const sstr &amt = itr->value.GetString();
        		if ( amt.size() > 30 ) {
        			continue;
        		}
        		if ( atof( amt.c_str() ) <= 0 ) {
        			i("E37023 error amt=[%s]", s(amt) );
        			return -45;
        		}
        		xferAmount = amt;
    		} else {
        		i("E31043 error not string" );
        		return -46;
    		}
		}

		d("a2221 xferAmount=[%s]", s(xferAmount));

		// deduct from tokens
		fromHasToken = false;
   		for ( rapidjson::SizeType j = 0; j < fromdom.Size(); ++j) {
			rapidjson::Value &v = fromdom[j];
			if ( ! v.IsObject() ) {
				continue;
			}
			itr = v.FindMember("name");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &tname = itr->value.GetString();
			if ( tname != xferName ) {
				continue;
			}

			itr = v.FindMember("bal");
			if ( itr == v.MemberEnd() ) {
				continue;
			}
			const sstr &bal = itr->value.GetString();

			if ( atof(bal.c_str()) < atof(xferAmount.c_str()) ) {
				i("E21722 error xferamount=[%s] iver bal=[%s]", xferAmount.c_str(), bal.c_str() );
				return -50;
			}

			fromHasToken = true;
			break;
		}

		if ( ! fromHasToken ) {
			//continue;  // do not touch todom
			i("E45023 fromacct has no such token [%s]", s(xferName) );
			return -55;
		}

		d("a22222\n");
	}

	return 0;
}


void t28()
{

    sstr cipher = "Gt$jh5j$k!CF'aDbR!sEW`IR,MFDQqgQAQJV5C(=Z\"/.DcR%EOK*G4gXUR(Z]!cpK%dR!L],Y:KF\"bl]&Dm3[g2C&``a@e1T/%iqaZ'quKoOWEps?lBEa`/0$L5mi*33k?T(qP^i0V$\\.()YB\\O7j2J@1l1?_*h^CC8IS@_9iX4Ml^6N5$^^laRPR&)0A*>ZT/_a0puUlFAI4InbBIaOMB2m-h:0%[Xlj]`D=V7qLtQSP)+p8?1Y'iM>PaX;]4,F\"4_UiGt:XF>3%>3L\"2U$nd-?ST>='LDl%iQO,oM^9$c[P\"B3%pjN)(rMpq:^=\\huqd(H1AOQKRqPA*!+JeGN(=Yt6?kNS6!k\\11qb_EC+gu#I!E,WLmm=c:*e4f4bK^eO0+$>J6/^e;:Z3W*>FT,_^?E@\"H_]Ek0^\\/Xjfe,R7p_J?'_leKZ]UN!L0V?_*;hQT;f:ik?2]&Y^Y4TJMbBWeSa:W=@Wh7d+fRKY[?\"V4`R/LD>6H!5Ma07tWq\"`(64a*5[.1$E]H1lr$auq@LU`WhIhe+p]:@N`CS\"jbk'4TAV@uL?pofS;a_:E]_3ck0(o[GUi2:]Y'.X#_/\"WEF4&6+S^'`gf^!3t6\"KI-Yn;2\"P=<Cc8JblXtC.U>,S!gBM38rUN8'=Mm^hY%)iPHU:NI7_hQuIjC^^O7A`QD#IO]j]J#'q\".k,=\\Yl$#lJ6+f.uR167[#6gao*2\"hLUeP'3Yb1p+\"OLh+\\sj;1],5Vn^C@8[7r7PX^^2,CA:L\"p53conb],pFR8YJsCBQbSH'!_cRH#`k1CR(c<l`KHhl#j:\\\",R'YUpp_eRc7Z_L/n.2.a2Es.Wt].B<:P*'TG.pk^j1b+bZ)1jBo0+fsC^TJ=X5_0kRnTR]Zh\\-(Th>KV>BImV3l5%;Z4\"d'7%8\"g(Uk-).9&*'CaArsAnl?9VMV\"KpA09\"kD7T\"/)fN@3J9bKmG`n!I'^8\\?*h\\cjs5*\"VhIm$b\\NZ]Ka@'L6S0W:UR1IJ9jAm>`eKCBT%Cqlk+!ikG,X<2]H9>%renX\"okq9.EBE58YeesR<X\"2UI>Tr:$VdRtM(^AE,Q?I=BfEY`?iD3k>)-_%?$]5/LYpjiKeak&\\f#$aK\"EN!$__rGF/-P_85#6g<NPrWeS9k6mYc362&Kb7ZLD&$Y8d?(m,E9>'0Pp\\;`MNLiQ2CjanrPBF$,hml0b'Gk=BfF?P]c?(@7n_:eI)C5o+q7?dQ.PsXjA(AH5%VO@m>.S&@NMk(RclAK]$2;W87M7sYD\\:H>:&L5-'?:qN>?A;LP=<V'/A%,DCk(Z')Zm9ln(r(,sNk4bO7r'puY5u4gSpN<o3X+oO_G>OsnT\"/3k)sk5n\"$[oqJs";


	sstr pubkey= "l`R3'HF=7:+m$/iQCf%uEsEe^\"Ue#&OFE\\u3*PM+l1)2S^H2XP@@\\$#e#+J:/<e#[G@K3-8%t^SBgV$`*db3as12(%QJ$bM1W-03:&PphNTIPn%RK<4O_=oO,fX*HSC*Er!r.XVpMq@0#!mA%q'W_$Bn=Q:-6Js(qS%'`4*e.WV+;f.\"fA/J=2OCQo6R$6Bqa(SB>M/:eA#qldN:E[#nE/!h<mosLT&G\\32*RSp8:DX91Z>oR+#TLbb6fI%oW9;#<X5:-m*d<1`Hc@$`<?&gt(%B3'Dj`*f):\\Q(O)86?QA+U[[\\_\"n:$l/=?Y`[f4.C:30irB-t-f-hLN>SJ]^UT:Wps-lLuFDjPM2$Y#&Dk^E@dm@n@,*S\"c$b&+]n;4PnWA-jGa(j6WY6cocgn!9<:V)h#67cEX1]KZQp?)TfmN!O<oaf14S.:qbN7Vu&C)hZ-T7IHn.iCpZ$Dad/;p[;L5YN+M)Vl5E>`\\'$]gqVa&QXiMuJ;b&6$,?<rpZe\"*X:Gu6_mliK`32(@<\\V6NVbf?sI[?T(^r3o(7h-No0.DY2Z9nG-Io\\?XFZU&CNbH67Q\\WHJgTs,)!ZY*pejKsY@4+k5rtL1g^WAJs)GmEj)A/]c7t>CE8J::)&3Mi5\\rOm\"1:5c<T\\#g)+g$2^$!4B$NB<q+GNN4-1pdgEcB&Jsb=:f3EM?<\"]j%HV%'8F/0KW_Tn\\\"!Ubl*^l&7NkA7ZZ\\gaQ;:<N%J</)Ml59rBc)<S!PQ5kp+te!oV?DDf2aX:eN!Y6l8$(8pQs\"E;KZaC/V[\"htUZrrNd<tgKQ^4=.5NW,=_p1c%h0k,Ot\"&9?S'1P1Sc.F^I8qD(O?c%[6-mn&BE8,B$5ar_tbOP44`Rno39=_Aikl?H-0jZMQtjX4F)'j/B*WnN4]rQ@<kcV9R5<\"SJm'iIK/P<f[[%cSGY%dZE3P&ZpTsC!T*3S?<?+P-J!V`mrKs:%g$^WW:bg<.6LLI/`0[f@?NMp,C4?Y#A'Q$0IC#4bqT)G%O3ALg6c_Z^Rr,X6aGtc#J=e87+NqT_@2T=XCg01usHk_CWKMYo5i5\\NL2P\\V-;Z1]ch*('h'.%!U>+ms\\^D+s4V+3e]I;fe8WA#$uP[lpn.=,Ji0NCF@5q?qId*BD+u\\0]t`F>SsTB^@s'p9']`(?5ZG9/\\BR8nFSWY%.WQD/HPZRc(E1P:N&A'_7tFZ#D`AXVe3YL,YHP`igr/_SGn,^WiP]<cS*::<\\9Mr";


  	sstr data="omtrxnid| 273515|user104||0|1650273515811324|A||{\"ACTYPE\":\"U\"}||||||||||";

	sstr signature="4F3S,RDr*AdVMbhU,[6;$\\][u57nLC";

 	sstr secretKey="!T<t[!!(sW\"[E%f!<57`!.Y(-s8W*!q#C?.s8W*\"&-)h5!!iQ+5QL_3!!!#uIfKHKrZ;.0IfL#[!5SU>!<57`!!*&VrrIZMn,NFfs8RQKnGiO'rso)0!!)rsrso)25QD>q!$D7E!!2Th!<<*\"!!+2B\"FpLa!!9t9s8W$^!;uisnG`N>rso,0561J.!9!nUIfKHKzJ,g,\\rr<!'!!*'\"r;Qa.!;aJ1#64kArrW6%i;WgC!#P\\9+92Op!:^$c^]+N=rZ;+CJH,ZLrI4eI!<6C+r-n]3!!7]N*ru=J!!E9&0E;*grs&N(_#OFa!:^!hJ,oBE!?_C;s8V!V!It.M!!3-##_2p5s8Tk6#64b^rso,0%fcT[rso)1+92NEzJ,oBEs$$J`J,o*=!rr<,!<(gT!rr>Zs8W-!#64cI!;uis&-)i`s7cNo_#FcB!9!kZ_#FW>s8W*(JH,*<\"b?U`!!*'\"!.b([!<3#u%fcYR!;q<H&-)jKrso)2+92Oprso)0_#FoFruV4@J,o*=!<<**!!D0Z#64`8!!%QL%fcVQ!;QTo0*)!$rso,056(Z`!\"],7JH,*<zJH$,[r'(/ez!!!!1!!E9%#QXfF!;q<Ii;`bj!!iQ*5QD&i!9!kX!!)Ng!?_@F!!*W2!^HbTrr=/A#_;q.!!L+;s8W+Kzi;WlZz+92Cl!!!#us8N<(!?_C?s8NT0r4`7,s8F)?r-n_izz!;q<H!!!'#!;QQo!!!:T!\"]/0IfK`S!+5g)rr=_Qrr<!7z\"98Gkrs&N(nGiJ0!;M$E&-2RA!!iQ*i;WlZ!!!#uIfK`Sra,X2JH%h6!WW3#!!48C\"9AK%rrYLd!!!#Wrs+)Shu<kqs7cQnhu<q3rZ;+AJ,fQL!?_@B!!!Q1rr<#urrF5B!.Y(]z!!*$@!!n,Tn,EL*rtbY85QD(?!\"]/,56(rhz!!*'\"!^H_sz\"FpIP!<3#u%fcSp!;M$FnGiIes7cNni;X18!\"]/05613QrZ;+GJH$,[r4i:+rr>:a!.b+Ls8RQK!!!(.!!!$!*ru/P!\"],2+9;?>s8W*\"z!!!!%J,g,\\r4i:3rr>:ao`\"mk!!5Cc#QXo)rs+)R0*)\"Os7cQn?N:'+ruV4B_#FoFr;Zd%!!*W2!!!#orrE*\"\"98G[rrYOd%fcV1!;q?H*ru;T!;QTn*rlQHrso,/56(rh!J(1XJH,*<\"$chl!!*'\"r;Zd-!<0b5#6=`f!\"8i-i;Wr\\!;QQoi;Wn0s6oskJH,ZLs'Gd$IfRjqs$$MIrrF5Br;Zd-!!'h7p](4W!;-9ki;`cUs8W*\"i;Wn0z!!)fo!$D7IJ,o*=#=&7p!!+2BpjW;%s8P:`nGiLfs8W-!*rl:+!;QQp+92NE!:^!h5QD&is$$M]IfU)\\\"98E-!<,1_\"98E5!!3-#nG`P4!!!!\"0ED*%z+92Op!\"],1J,fQLs$$MYIfU)\\r]^Ag!!'h7s8W,fs8Dutp\\t9p!;q?H%flP-rso,0s8N<(!:^$eIfL#[l`R3'HF=7:+m$/iQCf%uEsEe^\"Ue#&OFE\\u3*PM+l1)2S^H2XP@@\\$#e#+J:/<e#[G@K3-8%t^SBgV$`*db3as12(%QJ$bM1W-03:&PphNTIPn%RK<4O_=oO,fX*HSC*Er!r.XVpMq@0#!mA%q'W_$Bn=Q:-6Js(qS%'`4*e.WV+;f.\"fA/J=2OCQo6R$6Bqa(SB>M/:eA#qldN:E[#nE/!h<mosLT&G\\32*RSp8:DX91Z>oR+#TLbb6fI%oW9;#<X5:-m*d<1`Hc@$`<?&gt(%B3'Dj`*f):\\Q(O)86?QA+U[[\\_\"n:$l/=?Y`[f4.C:30irB-t-f-hLN>SJ]^UT:Wps-lLuFDjPM2$Y#&Dk^E@dm@n@,*S\"c$b&+]n;4PnWA-jGa(j6WY6cocgn!9<:V)h#67cEX1]KZQp?)TfmN!O<oaf14S.:qbN7Vu&C)hZ-T7IHn.iCpZ$Dad/;p[;L5YN+M)Vl5E>`\\'$]gqVa&QXiMuJ;b&6$,?<rpZe\"*X:Gu6_mliK`32(@<\\V6NVbf?sI[?T(^r3o(7h-No0.DY2Z9nG-Io\\?XFZU&CNbH67Q\\WHJgTs,)!ZY*pejKsY@4+k5rtL1g^WAJs)GmEj)A/]c7t>CE8J::)&3Mi5\\rOm\"1:5c<T\\#g)+g$2^$!4B$NB<q+GNN4-1pdgEcB&Jsb=:f3EM?<\"]j%HV%'8F/0KW_Tn\\\"!Ubl*^l&7NkA7ZZ\\gaQ;:<N%J</)Ml59rBc)<S!PQ5kp+te!oV?DDf2aX:eN!Y6l8$(8pQs\"E;KZaC/V[\"htUZrrNd<tgKQ^4=.5NW,=_p1c%h0k,Ot\"&9?S'1P1Sc.F^I8qD(O?c%[6-mn&BE8,B$5ar_tbOP44`Rno39=_Aikl?H-0jZMQtjX4F)'j/B*WnN4]rQ@<kcV9R5<\"SJm'iIK/P<f[[%cSGY%dZE3P&ZpTsC!T*3S?<?+P-J!V`mrKs:%g$^WW:bg<.6LLI/`0[f@?NMp,C4?Y#A'Q$0IC#4bqT)G%O3ALg6c_Z^Rr,X6aGtc#J=e87+NqT_@2T=XCg01usHk_CWKMYo5i5\\NL2P\\V-;Z1]ch*('h'.%!U>+ms\\^D+s4V+3e]I;fe8WA#$uP[lpn.=,Ji0NCF@5q?qId*BD+u\\0]t`F>SsTB^@s'p9']`(?5ZG9/\\BR8nFSWY%.WQD/HPZRc(E1P:N&A'_7tFZ#D`AXVe3YL,YHP`igr/_SGn,^WiP]<cS*::<\\9Mr\\p5!2^<W)0XRV52LH0\\u@!52C:7(QPhl#tbaoFK?ZLA\"i6XLDCbs`487!eg$RBd-C%H,Soc*t?$j`+sm";


    sstr hashPlain;
   	OmicroNodeKey::decryptSB3( signature, secretKey, cipher, hashPlain);

	printf("decryptSB3 hashPlain=[%s]\n", hashPlain.c_str() );

}

void t29()
{

	sstr pubkey = "l`R3'HF=7:+m$/iQCf%uEsEe^\"Ue#&OFE\\u3*PM+l1)2S^H2XP@@\\$#e#+J:/<e#[G@K3-8%t^SBgV$`*db3as12(%QJ$bM1W-03:&PphNTIPn%RK<4O_=oO,fX*HSC*Er!r.XVpMq@0#!mA%q'W_$Bn=Q:-6Js(qS%'`4*e.WV+;f.\"fA/J=2OCQo6R$6Bqa(SB>M/:eA#qldN:E[#nE/!h<mosLT&G\\32*RSp8:DX91Z>oR+#TLbb6fI%oW9;#<X5:-m*d<1`Hc@$`<?&gt(%B3'Dj`*f):\\Q(O)86?QA+U[[\\_\"n:$l/=?Y`[f4.C:30irB-t-f-hLN>SJ]^UT:Wps-lLuFDjPM2$Y#&Dk^E@dm@n@,*S\"c$b&+]n;4PnWA-jGa(j6WY6cocgn!9<:V)h#67cEX1]KZQp?)TfmN!O<oaf14S.:qbN7Vu&C)hZ-T7IHn.iCpZ$Dad/;p[;L5YN+M)Vl5E>`\\'$]gqVa&QXiMuJ;b&6$,?<rpZe\"*X:Gu6_mliK`32(@<\\V6NVbf?sI[?T(^r3o(7h-No0.DY2Z9nG-Io\\?XFZU&CNbH67Q\\WHJgTs,)!ZY*pejKsY@4+k5rtL1g^WAJs)GmEj)A/]c7t>CE8J::)&3Mi5\\rOm\"1:5c<T\\#g)+g$2^$!4B$NB<q+GNN4-1pdgEcB&Jsb=:f3EM?<\"]j%HV%'8F/0KW_Tn\\\"!Ubl*^l&7NkA7ZZ\\gaQ;:<N%J</)Ml59rBc)<S!PQ5kp+te!oV?DDf2aX:eN!Y6l8$(8pQs\"E;KZaC/V[\"htUZrrNd<tgKQ^4=.5NW,=_p1c%h0k,Ot\"&9?S'1P1Sc.F^I8qD(O?c%[6-mn&BE8,B$5ar_tbOP44`Rno39=_Aikl?H-0jZMQtjX4F)'j/B*WnN4]rQ@<kcV9R5<\"SJm'iIK/P<f[[%cSGY%dZE3P&ZpTsC!T*3S?<?+P-J!V`mrKs:%g$^WW:bg<.6LLI/`0[f@?NMp,C4?Y#A'Q$0IC#4bqT)G%O3ALg6c_Z^Rr,X6aGtc#J=e87+NqT_@2T=XCg01usHk_CWKMYo5i5\\NL2P\\V-;Z1]ch*('h'.%!U>+ms\\^D+s4V+3e]I;fe8WA#$uP[lpn.=,Ji0NCF@5q?qId*BD+u\\0]t`F>SsTB^@s'p9']`(?5ZG9/\\BR8nFSWY%.WQD/HPZRc(E1P:N&A'_7tFZ#D`AXVe3YL,YHP`igr/_SGn,^WiP]<cS*::<\\9Mr";

	//sstr msg = "89228303:xxh64hh";
	sstr msg = "omtrxnid| 326378|user106||0|1650326378888043|A||{\"ACTYPE\":\"U\"}||||||||||";

	sstr cipher, passwd, encMsg;

	OmicroNodeKey::encryptSB3( msg, pubkey, cipher, passwd, encMsg );

	printf("encMsg=[%s] size=%lu len=%lu\n", s(encMsg), encMsg.size(), strlen( encMsg.c_str() ) );

}

void t30()
{
	//sstr msg ="92492932:xxh64hh"; //fail encMsg empty, ok some cases
	//sstr msg ="924922"; //fail encMsg empty, ok some cases
	//sstr msg ="19249293283738383837339";
	//sstr msg ="1922093939393393939399393939393end";
	//sstr msg ="92492932:xxh6888888888888888888888888884hh-3-3939end"; 
	//sstr msg ="92492932:xxh64hh----end828383833838383838383838383833838383838838383838383833838||||||||||||||||||jjjdjdkjdkjdend"; // ok
	//sstr msg = "14193652717024615349"; // fail
	//sstr msg = "14193652717024615349:xxhash64"; // OK
	//sstr passwd = "@Fr[Y;fUR\"KaTg.hB*.6h4UQn&O\"<r^/UA!'aQ+#";  // fail
	//sstr passwd = "@Fr[Y;fUR\"KaTg.hB*.6h4UQn&O\"<r^/UA!'aQ+#000009999end";  // fail
	//sstr passwd = "@Fr[Y;fUR\"KaTg.hB*.6h4UQn&O\"<9end------------------------end2";  // fail
	// sstr passwd = "l#n$P<c6Jikc6X5?NI#K<t,`9W#Zm(Q,uPBS!n,:";
	//sstr passwd = "@Fr[Y;fURaTg.hB*.6h4UQn&O\"<r^/UA!'aQ+#";  // fail
	//sstr passwd = "@Fr[Y;fUR\"K"; // OK
	//sstr passwd = "@Fr[Y;fUR\"K--------"; // OK
	//sstr passwd = "9939@Fr[R----"; // OK
	//sstr passwd = "@Fr[Y;fUR\"KaTg.hB*.6h4UQn&O\"<r^/UA!'aQ+";  // fail
	//sstr passwd = "@Fr[Y;fUR\"KaTg.hB*.6h4UQn&O\"<r^/UA!'aQ";  // OK

	// fail
	//sstr msg = "5657761525880349042";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// ok
	//sstr msg = "5657349042";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// fail
	//sstr msg = "565776152588034904200";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// OK;
	//sstr msg = "hash:565776152588034904200";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// fail
	//sstr msg = "565776152588034904200:hash";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// OK
	//sstr msg = "xxhash:565776152588034904200";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// OK
	//sstr msg = "xy:5657761525880349042";
	//sstr passwd = "\"Xp>3+m*cl8rSg6;O!?-UC$!i^:!b>mJ@kL('_;B";

	// OK
	//sstr msg = "xyhsh:10357211318733233563";
	//sstr passwd = "1tbX;F]+5Y\\cFn0(op7;@9@e71$^37C`drQSj$^C";

	// OK
	//sstr msg = "xxhash:10357211318733233563";
	//sstr passwd = "1tbX;F]+5Y\\cFn0(op7;@9@e71$^37C`drQSj$^C";

	// fail
	//sstr msg = "xxhash:221442723108879392";
	//sstr passwd = "B3&sl3/d/*Sd93kr*3(`E=Lr`\\*J.]F1Llkfls*!";

	sstr msg = "xxhash:a43090d5c8916164:yy";
	sstr passwd = "!12Nur5)[EbXK.h,>>@87Nj:4gHUFOY>RLGMt/E$";

    printf(" msg=[%s] size=%lu\n", s(msg), msg.size() );

	sstr encMsg;
    aesEncrypt( msg, passwd, encMsg );
    printf("encMsg=[%s] size=%lu strlen=%lu\n", s(encMsg), encMsg.size(), strlen(encMsg.c_str()) );

    sstr plain;
    aesDecrypt( encMsg, passwd, plain );
    if ( plain != msg ) {
        printf("aesEncrypt/aesDecrypt problem\n");
        printf("msg  =[%s] msg.len=%lu\n", s(msg), msg.size() );
        printf("plain=[%s] pln.len=%lu\n", s(plain), plain.size() );
        printf("passwd=[%s] passwd.size=%lu strlen=%lu\n", s(passwd), passwd.size(), strlen(passwd.c_str()) );
    } else {
        printf("OK plain=[%s] pln.len=%lu\n", s(plain), plain.size() );
        printf("passwd=[%s] passwd.size=%lu strlen=%lu\n", s(passwd), passwd.size(), strlen(passwd.c_str()) );
	}
}

