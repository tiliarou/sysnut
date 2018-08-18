#include "server.h"
#include <stdio.h>
#include <errno.h>

#define REMOTE_INSTALL_PORT 2000

Server::Server()
{
	if (!socket().isOpen())
	{
		printf("Failed to create a server socket. Error code %u\n", errno);
		return;
	}


	if (socket().bind(REMOTE_INSTALL_PORT) < 0)
	{
		printf("Failed to bind to socket\n");
		return;
	}

	if (socket().listen() < 0)
	{
		printf("Failed to listen on server socket. Error code:\n");
		socket().close();
	}
}

Server::~Server()
{
}

bool Server::isActive()
{
	return true;
	//return appletMainLoop();
}

bool Server::run()
{
	if (!socket().isOpen())
	{
		return false;
	}

	while (true)
	{
		//printf("waiting for client...\n");

		auto callback = [](Socket *s) {
			s->write("hello world\n", sizeof("hello world\n"));
			printf("closing client\n");
		};
		
		socket().accept(callback);
	}

	return true;
}

void runServer()
{
	Server server;
	server.run();
}
