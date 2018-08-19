#include "server.h"
#include <switch.h>

extern "C"
{
	void runMainLoop();
}

void runMainLoop()
{
	while (appletMainLoop())
	{
		svcSleepThread(500000000L);
		Server s;
		for(int i=0; i < 10 && appletMainLoop() && s.isListening(); i++)
		{
			s.step();
			svcSleepThread(30000000L);
		}
	}
}