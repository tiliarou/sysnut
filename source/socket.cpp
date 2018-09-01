#include "socket.h"
#include <errno.h>
#include "log.h"

Socket* currentSocket = NULL;

Socket::Socket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
}

Socket::Socket(int socket)
{
	set(socket);
}

Socket::~Socket()
{
}

int Socket::operator=(int socket)
{
	set(socket);
	return socket;
}

void Socket::set(int socket)
{
	m_socket = socket;
}

bool Socket::close()
{
	if (isOpen())
	{
		::close(m_socket);
		m_socket = -1;
		return true;
	}
	return false;
}

bool Socket::isOpen()
{
	return m_socket >= 0;
}

int Socket::bind(int port)
{
	m_server.sin_family = AF_INET;
	m_server.sin_port = htons(port);
	m_server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (::bind(m_socket, (struct sockaddr*) &m_server, sizeof(m_server)) < 0)
	{
		close();
		return -1;
	}

	fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFL, 0) | O_NONBLOCK);
	return 1;
}

int Socket::listen(int queueMax)
{
	int r = ::listen(m_socket, queueMax);

	if (r)
	{
		close();
	}

	return r;
}

int Socket::accept(void(*callback)(Socket* s))
{
	struct sockaddr_in client;
	socklen_t clientLen = sizeof(client);

	Socket s(::accept(m_socket, (struct sockaddr*)&client, &clientLen));
	int r = s;

	if (s >= 0)
	{
		currentSocket = &s;
		callback(&s);
	}
	else if(errno != EAGAIN)
	{
		currentSocket = NULL;
		print("accept err: %d / %u\n", r, errno);
		s.close();
		close();
	}
	currentSocket = NULL;
	s.close();
	return r;
}

size_t Socket::write(const void* buf, size_t len)
{
	size_t blockSizeWritten = 0;
	size_t sizeWritten = 0;

	while ((sizeWritten += (blockSizeWritten = send(m_socket, (int8*)buf + sizeWritten, len - sizeWritten, 0)) < len) && (blockSizeWritten > 0))
	{
	}

	return sizeWritten;
}