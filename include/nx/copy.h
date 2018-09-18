#pragma once
#include "nx/buffer.h"
#include "nx/file.h"
#include "nx/lock.h"

class CopyBuffer
{
public:
	CopyBuffer() : lock()
	{
	}

	Buffer<u8>& buffer() { return m_buffer; }
	Lock lock;
private:
	Buffer<u8> m_buffer;
};



template<u32 BUFFER_SIZE=0x800000, u32 BUFFER_COUNT=3>
class Copy
{
public:
	Copy()
	{
		threadRunningCount = 0;

		totalSize = 0;
		bytesRead = 0;
		bytesWritten = 0;
		m_shouldRun = true;

		totalSize = 0;
	}

	~Copy()
	{
		print("killing thread\n");
		if (shouldRun())
		{
			m_shouldRun = false;
		}
#ifdef __SWITCH__
		threadWaitForExit(&t);
		threadClose(&t);
		print("thread died\n");
#endif
	}

	bool startWriterThread()
	{
#ifdef __SWITCH__
		memset(&ctx.t, 0, sizeof(ctx.t));
		if (threadCreate(&writerThreadContext, (void(*)(void*))&writerThread, this, 1 * 1024 * 1024, 0x2D, getNextCpuId()))
		{
			error("Failed to create download thread!\n");
			return false;
		}

		if (threadStart(&ctx.t))
		{
			error("Failed to start download thread!\n");
			return false;
		}
#else
		DWORD threadId;
		SECURITY_ATTRIBUTES   threadAttributes;
		ZeroMemory(&threadAttributes, sizeof(threadAttributes));
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&writerThread, this, 0, &threadId);
#endif
		return true;
	}

	bool start()
	{
		if (!startWriterThread())
		{
			return false;
		}

		readerThread(this);
		print("copy complete\n");
		return true;
	}

	static void WINAPI readerThread(Copy* ctx)
	{
		if (!ctx->readerInit())
		{
			return;
		}

		CopyBuffer* buffer = NULL;

		while (1)
		{
			buffer = ctx->buffers.peek();

			if (!(buffer->lock.acquireWriteLock()))
			{
				continue;
			}
			ctx->buffers.push();

			buffer->buffer().resize(0);

			if (!ctx->readChunk(ctx->bytesRead, buffer->buffer(), BUFFER_SIZE))
			{
				buffer->buffer().resize(0);
				buffer->lock.releaseWriteLock();
				break;
			}

			ctx->bytesRead += buffer->buffer().size();

			buffer->lock.releaseWriteLock();
		}

		ctx->readerExit();
	}

	static void WINAPI writerThread(Copy* ctx)
	{
		if (!ctx || !ctx->writerInit())
		{
			return;
		}

		CopyBuffer* buffer = NULL;

		ctx->bytesWritten = 0;
		ctx->threadRunningCount = true;

		while (ctx->shouldRun())
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
					ctx->writeChunk(ctx->bytesWritten, buffer->buffer());
					ctx->bytesWritten += buffer->buffer().size();
				}

				ctx->buffers.shift();
				buffer->lock.releaseReadLock();
			}
			else
			{
				nxSleep(100);
			}
		}
		ctx->writerExit();
		ctx->threadRunningCount--;
	}

	virtual u64 writeChunk(u64 offset, Buffer<u8>& buffer)
	{
		return buffer.size();
	}

	virtual bool writerInit()
	{
		return true;
	}

	virtual bool writerExit()
	{
		return true;
	}

	virtual u64 readChunk(u64 offset, Buffer<u8>& buffer, u64 sz)
	{
		buffer.resize(0);
		return sz;
	}

	virtual bool readerInit()
	{
		return true;
	}

	virtual bool readerExit()
	{
		return true;
	}

	bool shouldRun() { return m_shouldRun; }

	bool m_shouldRun;
	u64 threadRunningCount;

	NcaId ncaId;
	Thread writerThreadContext;
	Thread readerThreadContext;

	u64 totalSize;
	u64 bytesRead;
	u64 bytesWritten;

	ChineseSdBuffer<CopyBuffer, BUFFER_COUNT> buffers;
};

class FileCopy : public Copy<>
{
public:
	FileCopy(string src, string dst) : Copy<>()
	{
		fsrc = sptr<File>(new File());
		fsrc->open(src, "rb");

		fdst = sptr<File>(new File());
		fdst->open(dst, "wb");
	}

	u64 writeChunk(u64 offset, Buffer<u8>& buffer) override
	{
		print("writing %x\n", offset);
		fdst->write(buffer);
		print("write @ %x done\n", offset);
		return buffer.size();
	}

	u64 readChunk(u64 offset, Buffer<u8>& buffer, u64 sz) override
	{
		print("reading %x...\n", offset);
		sz = fsrc->read(buffer, sz);
		print("read @  %x done\n", offset);
		return sz;
	}

	bool writerInit() override
	{
		if (!fdst)
		{
			return false;
		}
		/*if (!fdst->isOpen())
		{
			fdst->open(dst.c_str(), "wb");
		}*/
		return fdst->isOpen();
	}

	bool writerExit() override
	{
		if (fdst)
		{
			//fdst->close();
		}
		return true;
	}

	bool readerInit() override
	{
		if (!fsrc)
		{
			return false;
		}
		/*if (!fsrc->isOpen())
		{
			fsrc->open(src.c_str(), "rb");
		}*/
		return fsrc->isOpen();
	}

	bool readerExit() override
	{
		if (fsrc)
		{
			//fsrc->close();
		}
		return true;
	}

	sptr<File> fsrc;
	sptr<File> fdst;
};