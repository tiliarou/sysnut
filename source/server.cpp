#include "server.h"
#include <stdio.h>
#include <errno.h>
#include "log.h"

#define REMOTE_INSTALL_PORT 2000

Server::Server()
{
	if (!socket().isOpen())
	{
		print("Failed to create a server socket. Error code %u\n", errno);
		return;
	}


	if (socket().bind(REMOTE_INSTALL_PORT) < 0)
	{
		print("Failed to bind to socket %u\n", errno);
		return;
	}

	if (socket().listen() < 0)
	{
		print("Failed to listen on server socket. Error code: %u\n", errno);
		socket().close();
	}
	else
	{
		print("listening\n");
	}
}

Server::~Server()
{
}

bool Server::isListening()
{
	return socket().isOpen();
}

bool Server::step()
{
	auto callback = [](Socket *s) {
		s->write("hello world\n", sizeof("hello world\n"));
		print("closing client\n");
	};

	socket().accept(callback);
	return true;
}

bool Server::run()
{
	if (!socket().isOpen())
	{
		return false;
	}

	while (true)
	{
		step();
	}

	return true;
}

