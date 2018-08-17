#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>
#include <sstream>

#include <memory>
#include <switch.h>
#include "server.h"



#define TITLE_ID 0x420000000000000E
#define HEAP_SIZE 0x000540000


extern "C" {
	extern u32 __start__;

	u32 __nx_applet_type = AppletType_None;

#define INNER_HEAP_SIZE 0x1000000
	size_t nx_inner_heap_size = INNER_HEAP_SIZE;
	char   nx_inner_heap[INNER_HEAP_SIZE];

	void __libnx_initheap(void);
	void __appInit(void);
	void __appExit(void);
}


void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	/* Newlib */
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end = (char*)addr + size;
}

void registerFspLr()
{
	if (kernelAbove400())
		return;

	Result rc = fsprInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);

	u64 pid;
	svcGetProcessId(&pid, CUR_PROCESS_HANDLE);

	rc = fsprRegisterProgram(pid, TITLE_ID, FsStorageId_NandSystem, NULL, 0, NULL, 0);
	if (R_FAILED(rc))
		fatalSimple(rc);
	fsprExit();
}


void __appInit(void)
{
	return;
	Result rc;
	rc = smInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);

	rc = fsInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);

	rc = fsdevMountSdmc();
	if (R_FAILED(rc))
		fatalSimple(rc);
}

void __appExit(void)
{
	return;
	fsdevUnmountAll();
	fsExit();
	smExit();
}

#define SOCK_BUFFERSIZE 16384

bool initNetwork()
{
	static const SocketInitConfig socketInitConfig = {
		.bsdsockets_version = 1,

		.tcp_tx_buf_size = 8 * SOCK_BUFFERSIZE,
		.tcp_rx_buf_size = 8 * SOCK_BUFFERSIZE,
		.tcp_tx_buf_max_size = 16 * SOCK_BUFFERSIZE,
		.tcp_rx_buf_max_size = 16 * SOCK_BUFFERSIZE,

		.udp_tx_buf_size = 0x2400,
		.udp_rx_buf_size = 0xA500,

		.sb_efficiency = 8,
	};

	Result ret = socketInitialize(&socketInitConfig);
	if (ret != 0)
	{
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	//mkdir("/logs", 0700);
	//unlink("/logs/sysnut.log");
	//stdout = stderr = fopen("/logs/sysnut.log", "a");
	printf("initializing\n");

#ifndef NXLINK_DEBUG
	socketInitializeDefault();
		/*if (!initNetwork())
		{
			printf("failed to initialize socket\n");
		}*/
#endif

	runServer();

	return 0;
}