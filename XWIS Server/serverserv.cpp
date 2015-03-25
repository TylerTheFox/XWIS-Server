#include "serverserv.hpp"

serverserv::serverserv() {	
	cout << "Serverserv Operating!" << endl;
	acceptor = new tcp::acceptor(service, tcp::endpoint(tcp::v4(), 4005));
	clientList = new clientList_ptr(new list<socket_ptr>);

	threads.create_thread(boost::bind<void>(&serverserv::acceptorLoop, this));
	boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::sml));

	threads.create_thread(boost::bind<void>(&serverserv::requestLoop, this));
	boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::sml));

	threads.join_all();
}

void serverserv::acceptorLoop()
{
	for (;;)
	{
		socket_ptr clientSock(new tcp::socket(service));
		acceptor->accept(*clientSock);
		mtx.lock();
		clientList->emplace_back(clientSock);
		mtx.unlock();
	}
}

void serverserv::requestLoop()
{
	for (;;)
	{
		if (!clientList->empty())
		{
			mtx.lock();
			for (auto& clientSock : *clientList)
			{
				if (clientSock->available())
				{
					char readBuf[1024] = { 0 };

					int bytesRead = clientSock->read_some(buffer(readBuf, 1024));

					string_ptr msg(new string(readBuf, bytesRead));

					if (clientSentExit(msg))
					{
						disconnectClient(clientSock);
						break;
					}


					if (isAskingForServerList(*msg)) {
						  clientSock->write_some(buffer(": 610 u 1\n", 10));
						  clientSock->write_some(buffer(": 605 u :xwis.brandanlasley.com 4003 '0:XWIS' -8 36.1083 -115.0582\n", 67));
						  clientSock->write_some(buffer(": 615 u :173.194.33.110 0 'US West Ping server' -8 36.1083 -115.0582\n", 69));
						  clientSock->write_some(buffer(": 607\n", 6));
					}
					disconnectClient(clientSock);
					break;
				}
			}
			mtx.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::millisec(sleepLen::lon));
	}
}

bool serverserv::isAskingForServerList(const std::string& s)
{
	if (s.find("QUIT") != std::string::npos)
	{
		return true;
	}
	return false;
}

char * serverserv::removeNullTerminator(char * str) {
	size_t len = strlen(str); // will calculate number of non-0 symbols before first 0
	char * output = (char *)malloc(len); // allocate memory for new array, don't forget to free it later
	memcpy(output, str, len); // copy data from old buf to new one
	return output;
}

bool serverserv::clientSentExit(string_ptr message)
{
	if (message->find("exit") != string::npos)
		return true;
	else
		return false;
}

void serverserv::disconnectClient(socket_ptr clientSock)
{
	auto position = find(clientList->begin(), clientList->end(), clientSock);
	clientSock->shutdown(tcp::socket::shutdown_both);
	clientSock->close();
	clientList->erase(position);
}