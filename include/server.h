#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "socket.h"


class Server
{
public:
	Server();
	~Server();

	bool run();
	bool isActive();
	Socket& socket() { return m_socket; }
private:
	Socket m_socket;
};

extern "C"
{
	void runServer();
}