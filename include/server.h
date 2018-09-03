#pragma once
#include "socket.h"


class Server
{
public:
	Server();
	~Server();

	bool run();
	bool step();
	bool isListening();
	Socket& socket() { return m_socket; }
private:
	Socket m_socket;
};
