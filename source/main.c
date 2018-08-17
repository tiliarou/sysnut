#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <switch.h>


#define TITLE_ID 0x420000000000000E
#define HEAP_SIZE 0x000540000
#define SOCK_BUFFERSIZE 16384

void runServer();

// we aren't an applet
u32 __nx_applet_type = AppletType_None;

// setup a fake heap
char fake_heap[HEAP_SIZE];

// we override libnx internals to do a minimal init
void __libnx_initheap(void)
{
	extern char *fake_heap_start;
	extern char *fake_heap_end;

	// setup newlib fake heap
	fake_heap_start = fake_heap;
	fake_heap_end = fake_heap + HEAP_SIZE;
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
	Result rc;
	svcSleepThread(10000000000L);
	rc = smInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);
	rc = fsInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);
	registerFspLr();
	rc = fsdevMountSdmc();
	if (R_FAILED(rc))
		fatalSimple(rc);
	rc = timeInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);
	rc = hidInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);
}

void __appExit(void)
{
	fsdevUnmountAll();
	fsExit();
	smExit();
	audoutExit();
	timeExit();
}

bool network_pre_init()
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

void network_post_exit(void)
{
	socketExit();
}


int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	network_pre_init();
	
	Thread serverThread;
	Result rc = threadCreate(&serverThread, runServer, NULL, 0x4000, 49, 3);

	if (R_FAILED(rc))
		fatalSimple(rc);

	rc = threadStart(&serverThread);

	if (R_FAILED(rc))
		fatalSimple(rc);

	while (appletMainLoop())
	{
		svcSleepThread(30000000L);
	}

	network_post_exit();

	return 0;
}