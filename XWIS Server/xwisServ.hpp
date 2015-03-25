#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996) // D_SCL_SECURE_NO_WARNINGS
#endif
#ifndef XWIS_H
#define XWIS_H

#include<iostream>
#include<list>
#include<map>
#include<queue>
#include<cstdlib>

#include<boost/asio.hpp>
#include<boost/thread.hpp>
#include<boost/asio/ip/tcp.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "irc_codes.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

class xwisServ {
public:
	enum serverType {
		serverserv,
		XWIS
	};
	xwisServ(int port, serverType _type);
private:
	boost::thread_group threads;
	typedef boost::shared_ptr<tcp::socket> socket_ptr;
	typedef boost::shared_ptr<string> string_ptr;
	typedef map<socket_ptr, string_ptr> clientMap;
	typedef boost::shared_ptr<clientMap> clientMap_ptr;
	typedef boost::shared_ptr< list<socket_ptr> > clientList_ptr;
	io_service service;
	tcp::acceptor * acceptor;
	clientList_ptr * clientList;
	boost::mutex mtx;
	enum sleepLen // Time is in milliseconds
	{
		sml = 100,
		lon = 200
	};
	serverType type;


	bool isAskingForServerList(const std::string& s);


	// Processing Stuff
	string xwisServ::getTime();

	// Server Stuff
	bool clientSentExit(string_ptr);
	void disconnectClient(socket_ptr);
	void acceptorLoop();
	void requestLoop();
};

#endif