#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define int8 unsigned char

class Server
{
public:
	Server();
	~Server();

	bool run();
	bool close();
	bool isOpen();
	size_t write(const void* buf, size_t len);
private:
	int m_serverSocket = 0;
	int m_clientSocket = 0;
};

extern "C"
{
	void runServer();
}