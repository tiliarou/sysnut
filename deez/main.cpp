#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <switch.h>
#include "nx/ipc/es.h"
#include "nx/ipc/ns_ext.h"
#include "nx/ipc/ncm_ext.h"


#define SOCK_BUFFERSIZE 16384

void runMainLoop();

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

	rc = fsdevMountSdmc();
	if (R_FAILED(rc))
		fatalSimple(rc);

	rc = timeInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);

	rc = hidInitialize();
	if (R_FAILED(rc))
		fatalSimple(rc);

	if (R_FAILED(esInitialize()))
		fatalSimple(0xBEE5);

	if (R_FAILED(splInitialize()))
		fatalSimple(0xBEE6);

	if (R_FAILED(splCryptoInitialize()))
		fatalSimple(0xBEE7);



	if (R_FAILED(ncmextInitialize()))
		fatalSimple(0xBEEF);

	if (R_FAILED(ncmInitialize()))
		fatalSimple(0xBEE2);

	if (R_FAILED(nsInitialize()))
		fatalSimple(0xBEE3);

	if (R_FAILED(nsextInitialize()))
		fatalSimple(0xBEE4);

	if (R_FAILED(nifmInitialize()))
		fatalSimple(0xBEE6);
}

void __appExit(void)
{
	nifmExit();
	ncmextExit();
	ncmExit();
	nsExit();
	nsextExit();

	splCryptoExit();
	splExit();
	esExit();
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
	
	gfxInitDefault();
	consoleInit(NULL);

	network_pre_init();
	
	while(appletMainLoop())
    {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
		{
			break;
		}

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    gfxExit();
	
	network_post_exit();

	return 0;
}