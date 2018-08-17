#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>
#include <sstream>

#include <memory>
#include <switch.h>



#define TITLE_ID 0x420000000000000E
#define HEAP_SIZE 0x000540000

const unsigned int MAX_URL_SIZE = 1024;
const unsigned int MAX_URLS = 256;
const int REMOTE_INSTALL_PORT = 2000;
static int m_serverSocket = 0;
static int m_clientSocket = 0;


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
	fsdevUnmountAll();
	fsExit();
	smExit();
}

void initializeServerSocket()
{
	printf("initializeServerSocket()\n");
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (m_serverSocket < -1)
	{
		printf("Failed to create a server socket. Error code: %u\n", errno);
	}
	else
	{

		struct sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(REMOTE_INSTALL_PORT);
		server.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(m_serverSocket, (struct sockaddr*) &server, sizeof(server)) < 0)
		{
			printf("Failed to bind server socket. Error code: %u\n", errno);
		}
		else
		{
			fcntl(m_serverSocket, F_SETFL, fcntl(m_serverSocket, F_GETFL, 0) | O_NONBLOCK);

			if (listen(m_serverSocket, 5) < 0)
			{
				printf("Failed to listen on server socket. Error code: %u\n", errno);
			}
			else
			{
				// success
				return;
			}
		}
	}

	if (m_serverSocket != 0)
	{
		close(m_serverSocket);
		m_serverSocket = 0;
	}
}

size_t WaitReceiveNetworkData(int sockfd, void* buf, size_t len)
{
	size_t blockSizeRead = 0;
	size_t sizeRead = 0;

	while ((sizeRead += (blockSizeRead = recv(sockfd, (u8*)buf + sizeRead, len - sizeRead, 0)) < len) && (blockSizeRead > 0 || errno == EAGAIN))
	{
		errno = 0;
	}

	return sizeRead;
}

size_t WaitSendNetworkData(int sockfd, const void* buf, size_t len)
{
	size_t blockSizeWritten = 0;
	size_t sizeWritten = 0;

	while ((sizeWritten += (blockSizeWritten = send(sockfd, (u8*)buf + sizeWritten, len - sizeWritten, 0)) < len) && (blockSizeWritten > 0 || errno == EAGAIN))
	{
		errno = 0;
	}

	return sizeWritten;
}

void runServer()
{
	// Initialize the server socket if it hasn't already been
	if (m_serverSocket == 0)
	{
		initializeServerSocket();

		if (m_serverSocket <= 0)
		{
			close(m_serverSocket);
			m_serverSocket = 0;
			return;
		}
	}

	while (true)
	{
		struct sockaddr_in client;
		socklen_t clientLen = sizeof(client);

		printf("waiting for client...\n");
		m_clientSocket = accept(m_serverSocket, (struct sockaddr*)&client, &clientLen);
		printf("client accepted\n");
		if (m_clientSocket >= 0)
		{
			printf("client good\n");
			u32 size = 0;
			WaitSendNetworkData(m_clientSocket, "hello world", sizeof("hello world"));
			WaitReceiveNetworkData(m_clientSocket, &size, sizeof(u32));
			size = ntohl(size);


			if (size > MAX_URL_SIZE * MAX_URLS)
			{
				//THROW_FORMAT("URL size %x is too large!\n", size);
				break;
			}

			char buf[2056];

			WaitReceiveNetworkData(m_clientSocket, buf, 2056);

			WaitSendNetworkData(m_clientSocket, "hello world", sizeof("hello world"));
			printf("closing client\n");
				
		}
		else if (errno != EAGAIN)
		{
			printf("Failed to open client socket with code %u\n", errno);
			break;
		}
	}


	if (m_clientSocket != 0)
	{
		close(m_clientSocket);
		m_clientSocket = 0;
	}
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
	//socketInitializeDefault()
	if (!initNetwork())
	{
		printf("failed to initialize socket\n");
	}
#endif

	while (true)
	{
		printf("starting server\n");
		runServer();
		if (m_serverSocket != 0)
		{
			close(m_serverSocket);
			m_serverSocket = 0;
		}
		svcSleepThread(1000000000L);
	}

	return 0;
}