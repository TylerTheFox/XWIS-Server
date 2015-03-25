#include<iostream>
#include<cstdlib>

#include<boost/thread.hpp>
#include "xwisServ.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
void startServerserv();
void startXwis();

xwisServ * serv;
xwisServ * xwis;
int main(int argc, char** argv)
{
	boost::thread ss(boost::bind<void>(startServerserv));
	boost::this_thread::sleep(boost::posix_time::millisec(200));
	boost::thread xs(boost::bind<void>(startXwis));
	puts("C++ XWIS, dont touch.");
	getc(stdin);
	return EXIT_SUCCESS;
}

void startServerserv() {
	serv = new xwisServ(4005, xwisServ::serverserv);
}

void startXwis() {
	xwis = new xwisServ(4003, xwisServ::XWIS);
}