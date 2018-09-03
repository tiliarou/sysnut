#include "server.h"
#include <stdio.h>
#include "log.h"


#define REMOTE_INSTALL_PORT 2000

Server::Server()
{
	if (!socket().isOpen())
	{
		print("Failed to create a server socket. Error code %u, socket %d\n", socket().errorCode(), (int)socket());
		return;
	}


	if (socket().bind(REMOTE_INSTALL_PORT) < 0)
	{
		print("Failed to bind to socket %u\n", socket().errorCode());
		return;
	}

	if (socket().listen() < 0)
	{
		print("Failed to listen on server socket. Error code: %u\n", socket().errorCode());
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
		
		/*tin::install::nsp::NetworkNSPInstallTask task(FsStorageId_SdCard, false, "guest:guest@192.168.254.11:9000/api/download/01009CE00AFAE000/title.nsp");

		if (task.PrepareForInstall() && task.Install())
		{
			print("success\n");
			// success
		}*/
							
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

