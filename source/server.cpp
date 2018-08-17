#include "server.h"
#include <stdio.h>
#include <errno.h>

#define REMOTE_INSTALL_PORT 2000

Server::Server()
{
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (m_serverSocket < 0)
	{
		printf("Failed to create a server socket. Error code %u\n", errno);
		return;
	}


	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(REMOTE_INSTALL_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_serverSocket, (struct sockaddr*) &server, sizeof(server)) < 0)
	{
		printf("Failed to bind server socket. Error code:\n");
		close();
		return;
	}

	fcntl(m_serverSocket, F_SETFL, fcntl(m_serverSocket, F_GETFL, 0) & ~O_NONBLOCK);

	if (listen(m_serverSocket, 5) < 0)
	{
		printf("Failed to listen on server socket. Error code:\n");
		close();
	}
}

Server::~Server()
{
	close();
}

bool Server::isOpen()
{
	return m_serverSocket > 0;
}

bool Server::close()
{
	if (m_serverSocket < 1)
	{
		::close(m_serverSocket);
		m_serverSocket = 0;
		return true;
	}
	return false;
}

bool Server::run()
{
	if (!isOpen())
	{
		return false;
	}

	while (true)
	{
		struct sockaddr_in client;
		socklen_t clientLen = sizeof(client);

		printf("waiting for client...\n");
		m_clientSocket = accept(m_serverSocket, (struct sockaddr*)&client, &clientLen);
		printf("client accepted\n");

		if (m_clientSocket >= 0)
		{
			write("hello world", sizeof("hello world"));
			printf("closing client\n");
			::close(m_clientSocket);
		}
		else
		{
			printf("Failed to open client socket with code\n");
			break;
		}
	}

	return true;
}

size_t Server::write(const void* buf, size_t len)
{
	size_t blockSizeWritten = 0;
	size_t sizeWritten = 0;

	while ((sizeWritten += (blockSizeWritten = send(m_clientSocket, (int8*)buf + sizeWritten, len - sizeWritten, 0)) < len) && (blockSizeWritten > 0))
	{
	}

	return sizeWritten;
}

void runServer()
{
	Server server;
	server.run();
}
