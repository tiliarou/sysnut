#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define int8 unsigned char

class Socket;


class Socket
{
public:
	Socket();
	Socket(int socket);
	~Socket();
	void set(int socket);
	int operator=(int socket);
	operator int() { return m_socket;  }
	int bind(int port);
	int accept(void(*callback)(Socket* s));
	bool close();
	bool isOpen();
	int listen(int queueMax=5);
	size_t write(const void* buf, size_t len);
private:
	int m_socket = 0;
	struct sockaddr_in m_server;
};