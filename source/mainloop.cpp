#include "server.h"
#include <switch.h>

extern "C"
{
	void runMainLoop();
}

void runMainLoop()
{
	Server s;
	while (appletMainLoop())
	{
		s.step();
		svcSleepThread(30000000L);
	}
}