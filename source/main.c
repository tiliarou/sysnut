#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <switch.h>
#include "nx/ipc/tin_ipc.h"
#include "nx/ipc/spl.h"
#include "error.hpp"


#define TITLE_ID 0x420000000000000E
#define HEAP_SIZE 0x000540000
#define SOCK_BUFFERSIZE 16384

void runMainLoop();

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
	if (R_FAILED(esInitialize()))
		fatalSimple(0xBEE5);

	if (R_FAILED(splInitialize()))
		fatalSimple(0xBEE6);

	if (R_FAILED(splCryptoInitialize()))
		fatalSimple(0xBEE7);
}

void __appExit(void)
{
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

/*
typedef struct {
	u8 c[0x10];
} RightsId;
*/

static const uint8_t g_titlekey_seal_key_source[0x10] = {
	0xCB, 0xB7, 0x6E, 0x38, 0xA1, 0xCB, 0x77, 0x0F, 0xB2, 0xA5, 0xB2, 0x9D, 0xD8, 0x56, 0x9F, 0x76
};

void dumpTickets()
{
	RightsId* rightsIds = NULL;
	u32 rightsIdCount = 0;
	u32 installedTicketCount = 0;
	FILE* f = fopen("/titlekeys.txt", "w+");
	u8 titleKey[16];
	u8 kek[16];
	u32 cryptoHandle = 0;

	splCryptoLockAesEngine(&cryptoHandle);

	memset(kek, NULL, sizeof(kek));
	splCryptoGenerateAesKek(g_titlekey_seal_key_source, 0, 0, kek);

	for (unsigned int j = 0; j < sizeof(kek); j++)
	{
		fprintf(f, "%02X", kek[j]);
	}

	fprintf(f, "\n");

	ASSERT_OK(esCountPersonalizedTicket(&installedTicketCount), "Failed to count personalized tickets");

	rightsIds = malloc(sizeof(RightsId) * installedTicketCount);
	memset(rightsIds, NULL, sizeof(RightsId) * installedTicketCount);

	ASSERT_OK(esListPersonalizedTicket(&rightsIdCount, rightsIds, installedTicketCount * sizeof(RightsId)), "Failed to list personalized tickets");

	for (unsigned int i = 0; i < rightsIdCount; i++)
	{
		for (unsigned int j = 0; j < sizeof(RightsId); j++)
		{
			fprintf(f, "%02X", rightsIds[i].c[j]);
		}
		fprintf(f, "|");

		memset(&titleKey, NULL, sizeof(titleKey));
		ASSERT_OK(esGetTitleKey(&rightsIds[i], &titleKey, sizeof(titleKey)), "Failed to get title key");

		for (unsigned int j = 0; j < sizeof(titleKey); j++)
		{
			fprintf(f, "%02X", titleKey[j]);
		}

		fprintf(f, "\n");
	}

	free(rightsIds);
	fprintf(f, "\n");


	ASSERT_OK(esCountCommonTicket(&installedTicketCount), "Failed to count common tickets");

	rightsIds = malloc(sizeof(RightsId) * installedTicketCount);
	memset(rightsIds, NULL, sizeof(RightsId) * installedTicketCount);

	ASSERT_OK(esListCommonTicket(&rightsIdCount, rightsIds, installedTicketCount * sizeof(RightsId)), "Failed to list common tickets");

	for (unsigned int i = 0; i < rightsIdCount; i++)
	{
		for (unsigned int j = 0; j < sizeof(RightsId); j++)
		{
			fprintf(f, "%02X", rightsIds[i].c[j]);
		}
		fprintf(f, "|");

		memset(&titleKey, NULL, sizeof(titleKey));
		ASSERT_OK(esGetTitleKey(&rightsIds[i], &titleKey, sizeof(titleKey)), "Failed to get title key");

		for (unsigned int j = 0; j < sizeof(titleKey); j++)
		{
			fprintf(f, "%02X", titleKey[j]);
		}

		fprintf(f, "\n");
	}

	free(rightsIds);

	fclose(f);

	splCryptoUnlockAesEngine(cryptoHandle);
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	//dumpTickets();

	network_pre_init();
	runMainLoop();
	network_post_exit();

	return 0;
}