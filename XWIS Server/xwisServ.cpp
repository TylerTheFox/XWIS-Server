#include "xwisServ.hpp"

xwisServ::xwisServ(int port, serverType _type) {
	type = _type;
	acceptor = new tcp::acceptor(service, tcp::endpoint(tcp::v4(), port));
	clientList = new clientList_ptr(new list<socket_ptr>);

	threads.create_thread(boost::bind<void>(&xwisServ::acceptorLoop, this));
	boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::sml));

	threads.create_thread(boost::bind<void>(&xwisServ::requestLoop, this));
	boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::sml));

	threads.join_all();
}

xwisServ::~xwisServ(){
	threads.interrupt_all();
	delete acceptor;
	delete clientList;
}

void xwisServ::acceptorLoop()
{
	while (true)
	{
		socket_ptr clientSock(new tcp::socket(service));
		acceptor->accept(*clientSock);
		mtx.lock();
		(*clientList)->emplace_back(clientSock);
		mtx.unlock();
		boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::sml));
	}
}

void xwisServ::requestLoop()
{
	while (true)
	{
		if (!(*clientList)->empty())
		{
			mtx.lock();
			for (auto& clientSock : *(*clientList))
			{
				if (clientSock->available())
				{
					boost::asio::streambuf response;
					boost::asio::read_until(*clientSock, response, "\r\n\0");

					string tmp;
					std::ostringstream ss;
					ss << &response;
					tmp = ss.str();


					string_ptr msg(new string(tmp));

					if (clientSentExit(msg))
					{
						disconnectClient(clientSock);
						break;
					}


					if (type == XWIS) {

						if ((*msg).find("verchk") != std::string::npos) {
							clientSock->write_some(buffer(": 375 u :- Welcome to Expansive Civilian Warfare!\n", strlen(": 375 u : -Welcome to Expansive Civilian Warfare!\n")));
							clientSock->write_some(buffer(": 372 u :- \n", strlen(": 372 u :- \n")));
							clientSock->write_some(buffer(": 372 u :- This is an ultra secret message of the day :)\n", strlen(": 372 u :- This is an ultra secret message of the day :)\n")));
							clientSock->write_some(buffer(": 376 u\n", strlen(": 376 u\n")));
							clientSock->write_some(buffer(": 379 u :none none none 1 32512 NONREQ\n", strlen(": 379 u :none none none 1 32512 NONREQ\n")));
						}
						else if ((*msg).find("TIME") != std::string::npos) {
							clientSock->write_some(buffer(": 391 u irc.brandanlasley.com :", strlen(": 391 u irc.brandanlasley.com :")));
							clientSock->write_some(buffer(getTime(), strlen(getTime().c_str())));
							clientSock->write_some(buffer("\n", 1));
						}
						else if ((*msg).find("SETCODEPAGE") != std::string::npos) {
							clientSock->write_some(buffer(": 329 u 1252\n", strlen(": 329 u 1252\n")));
						}
						else if ((*msg).find("GETINSIDER") != std::string::npos) {
							clientSock->write_some(buffer(": 399 u ECW`0\n", strlen(": 399 u ECW`0\n")));
						}
						else if ((*msg).find("JOINGAME") != std::string::npos) {
							vector<string> strs;
							boost::split(strs, (*msg), boost::is_any_of(" "));
							clientSock->write_some(buffer(":ECW!u\\@h JOINGAME ", strlen(":ECW!u\\@h JOINGAME ")));
							clientSock->write_some(buffer(strs[2], strlen(strs[2].c_str())));
							clientSock->write_some(buffer(" ", 1));
							clientSock->write_some(buffer(strs[3], strlen(strs[3].c_str())));
							clientSock->write_some(buffer(" ", 1));
							clientSock->write_some(buffer(strs[4], strlen(strs[4].c_str())));
							clientSock->write_some(buffer(" 0 ", 3));
							clientSock->write_some(buffer(strs[5], strlen(strs[5].c_str())));
							clientSock->write_some(buffer(" ", 1));
							clientSock->write_some(buffer(" 0 ", 1));
							clientSock->write_some(buffer(" ", 1));
							clientSock->write_some(buffer(strs[6], strlen(strs[6].c_str())));
							clientSock->write_some(buffer(" :#", 3));
							clientSock->write_some(buffer(" #ECW\n", 6));
						}
						else if ((*msg).find("CODEPAGE") != std::string::npos) {
							// todo
						}
						else if ((*msg).find("GETLOCALE") != std::string::npos) {
							// todo
						}
						else if ((*msg).find("GETCODEPAGE") != std::string::npos) {
							// todo
						}
						else if ((*msg).find("STARTG") != std::string::npos) {
							clientSock->write_some(buffer(":ECW!u\\@h STARTG u :ECW 0.0.0.0 :1650524 ", strlen(":ECW!u\\@h STARTG u :ECW 0 :1650524 ")));
							clientSock->write_some(buffer(getTime(), strlen(getTime().c_str())));
							clientSock->write_some(buffer("\n", 1));
						}
					}
					else {
						if (isAskingForServerList(*msg)) {
							clientSock->write_some(buffer(": 610 u 1\n", 10));
							clientSock->write_some(buffer(": 605 u :xwis.brandanlasley.com 4003 '0:XWIS' -8 36.1083 -115.0582\n", 67));
							clientSock->write_some(buffer(": 615 u :173.194.33.110 0 'US West Ping server' -8 36.1083 -115.0582\n", 69));
							clientSock->write_some(buffer(": 607\n", 6));
						}
					}
				}
			}
		}
		mtx.unlock();
		boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::lon));
	}
}

bool xwisServ::isAskingForServerList(const std::string& s)
{
	if (s.find("QUIT") != std::string::npos)
	{
		return true;
	}
	return false;
}

string xwisServ::getTime() {
	std::time_t result = std::time(nullptr);
	return std::to_string(result);
}

bool xwisServ::clientSentExit(string_ptr message)
{
	if (message->find("exit") != string::npos)
		return true;
	else
		return false;
}

void xwisServ::disconnectClient(socket_ptr clientSock)
{
	auto position = find((*clientList)->begin(), (*clientList)->end(), clientSock);
	clientSock->shutdown(tcp::socket::shutdown_both);
	clientSock->close();
	(*clientList)->erase(position);
}