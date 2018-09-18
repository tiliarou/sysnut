#pragma once
#include "nx/cnmt.h"
#include "nx/ipc/ns_ext.h"
#include "nx/contentstorage.h"
#include "nx/buffer.h"
#include "nx/chinesesdbuffer.h"
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
		this->install = install;
		this->ncaId = ncaId;
		threadInit = false;
		threadRunning = false;
		i = 0;
	}

	~InstallThreadContext()
	{
		print("killing thread\n");
		if (threadRunning)
		{
			threadRunning = false;
		}
#ifdef __SWITCH__
		threadWaitForExit(&t);
		threadClose(&t);
		print("thread died\n");
#endif
	}

	static void writerThread(InstallThreadContext* ctx)
	{
		if (!ctx || !ctx->install)
		{
			return;
		}

		InstallBuffer* buffer = NULL;
		u64 offset = 0;
		ctx->i = 0;
		ctx->threadInit = true;
		ctx->threadRunning = true;

		while (ctx->threadRunning)
		{
			if (ctx->buffers.size())
			{
				if (!(buffer = ctx->buffers.first()))
				{
					continue;
				}

				if (!buffer->lock.acquireReadLock())
				{
					continue;
				}

				if (buffer->buffer().size())
				{
					//ctx->install->storage.writePlaceholder(ctx->ncaId, offset, buffer->buffer().buffer(), buffer->buffer().size());
					offset += buffer->buffer().size();
				}

				ctx->buffers.shift();
				buffer->lock.releaseReadLock();
				ctx->i++;
			}
			else
			{
				nxSleep(100);
			}
		}
		ctx->threadRunning = false;
	}

	bool threadInit;
	bool threadRunning;

	Install* install;
	File* nca;
	NcaId ncaId;
	Thread t;
	u64 i;
	u64 totalSize;

	ChineseSdBuffer<InstallBuffer, 3> buffers;
};

class Install
{
public:
	Install(Directory* dir, Nca* cnmtNca, Cnmt* cnmt);
	bool install();
	bool installNca(File* nca, NcaId ncaId);
	bool installContentMetaRecords(Buffer<u8> installContentMetaBuf);
	bool installApplicationRecord();

	static void writerThread(InstallThreadContext* ctx = NULL);

//private:
	Nca* cnmtNca;
	Cnmt* cnmt;
	ContentStorage storage;
	Directory* dir;
};
