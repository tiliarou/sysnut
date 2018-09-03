#include "socket.h"
#include <errno.h>
#include "log.h"

Socket* currentSocket = NULL;
bool isNetworkInitialized = false;

bool initializeNetwork()
{
	if (isNetworkInitialized)
	{
		return true;
	}

#ifdef _MSC_VER
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0)
	{
		print("WSAStartup failed with error: %d\n", result);
		return false;
	}
#endif
	isNetworkInitialized = true;
	return true;
}

Socket::Socket()
{
	initializeNetwork();
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
		::closesocket(m_socket);
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
#ifdef _MSC_VER
	u_long mode = 1;
	ioctlsocket(m_socket, FIONBIO, &mode);
#else
	fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFL, 0) | O_NONBLOCK);
#endif
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
	else if(!errorWouldBlock())
	{
		currentSocket = NULL;
		print("accept err: %d / %u\n", r, errorCode());
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

	while ((sizeWritten += (blockSizeWritten = send(m_socket, (char*)buf + sizeWritten, len - sizeWritten, 0)) < len) && (blockSizeWritten > 0))
	{
	}

	return sizeWritten;
}

int Socket::errorCode()
{
#ifdef _MSC_VER
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool Socket::errorWouldBlock()
{
#ifdef _MSC_VER
	return WSAGetLastError() == WSAEWOULDBLOCK;
#else
	return errno == EAGAIN;
#endif
}