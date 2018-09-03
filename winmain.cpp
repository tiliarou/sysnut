#include <stdio.h>
#include "server.h"

bool appletMainLoop()
{
	return true;
}

int main()
{
	Server server;

	server.run();
}
