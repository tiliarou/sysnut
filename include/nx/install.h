#pragma once
#include "nx/cnmt.h"
#include "nx/ipc/ns_ext.h"
#include "nx/contentstorage.h"
#include "nx/buffer.h"
#include "nx/circularbuffer.h"
#include "nx/lock.h"

class cnmt;
class storage;
class dir;
class Nca;

string getBaseTitleName(TitleId baseTitleId);

class InstallBuffer
{
public:
	InstallBuffer() : lock()
	{
	}

	Buffer<u8>& buffer() { return m_buffer; }
	Lock lock;
private:
	Buffer<u8> m_buffer;
};

class Install;

class InstallThreadContext
{
public:
	InstallThreadContext(Install* install, File* nca, NcaId ncaId)
	{
		threadInit = false;
		threadRunning = true;
	}

	~InstallThreadContext()
	{
		threadRunning = false;
#ifdef __SWITCH__
		svcSleepThread(_1MS * 10000);
#endif
	}

	bool threadInit;
	bool threadRunning;

	Install* install;
	File* nca;
	NcaId ncaId;
	Thread t;

	CircularBuffer<InstallBuffer, 5> buffers;
};

class Install
{
public:
	Install(Directory* dir, Nca* cnmtNca, Cnmt* cnmt);
	bool install();
	bool installNca(File* nca, NcaId ncaId);
	bool installContentMetaRecords(Buffer<u8> installContentMetaBuf);
	bool installApplicationRecord();

	static void installThread(InstallThreadContext* ctx = NULL);

private:
	Nca* cnmtNca;
	Cnmt* cnmt;
	ContentStorage storage;
	Directory* dir;
};
