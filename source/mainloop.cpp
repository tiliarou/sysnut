#include "server.h"
#include <stdio.h>
#include <switch.h>

extern "C"
{
	void runMainLoop();
}

void runMainLoop()
{
	stdout = stderr = fopen("/sysnut.log", "a");
	printf("start up\n");
	while (appletMainLoop())
	{
		printf("loop start\n");
		svcSleepThread(500000000L);
		Server s;
		while(appletMainLoop() && s.isListening())
		{
			s.step();
			svcSleepThread(30000000L);
		}
		printf("loop end\n");
	}
	printf("shutdown");
}