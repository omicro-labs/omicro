#include <string>
#include <iostream>
#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>

#include "omutil.h"
#include "omicroclient.h"
#include "omicrotrxn.h"

INIT_LOGGING

int main(int argc, char* argv[])
{
	std::cout << "a393939 OmicroClient ..." << std::endl;
	//d("a02029 OmicroClient ...");
	OmicroClient client( argv[1], atoi(argv[2]) );
	//d("a02029 OmicroClient done");
	std::cout << "a393939 OmicroClient done." << std::endl;

	OmicroTrxn t1;
	std::cout << "a393939 t1.makeDummyTrxn." << std::endl;
	t1.makeDummyTrxn();
	std::cout << "a393939 t1.setInitTrxn." << std::endl;
	t1.setInitTrxn();
	std::cout << "a393939 t1.setInitTrxn done" << std::endl;
	std::string reply = client.sendMessage( t1.str(), true );
	std::cout << reply << std::endl;

	/**
	OmicroTrxn t2;
	t2.makeDummyTrxn();
	//t2.setInitTrxn();
	reply = client.sendMessage( t2.str(), 100 );
	std::cout << reply << std::endl;
	**/
}

