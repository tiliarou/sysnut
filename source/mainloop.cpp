#include "server.h"
#include <stdio.h>
#include <switch.h>
#include "log.h"

extern "C"
{
	void runMainLoop();
}

void runMainLoop()
{
	print("start up\n");
	while (appletMainLoop())
	{
		print("loop start\n");
		svcSleepThread(500000000L);
		Server s;
		while(appletMainLoop() && s.isListening())
		{
			s.step();
			svcSleepThread(30000000L);
		}
		print("loop end\n");
	}
	print("shutdown\n");
}