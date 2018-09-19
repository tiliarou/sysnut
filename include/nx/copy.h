#pragma once
#include "nut.h"
#include "nx/buffer.h"
#include "nx/file.h"
#include "nx/lock.h"
#include "nx/thread2.h"

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



class Copy : public Thread2
{
public:
	Copy() : Thread2()
	{
		totalSize = 0;
		bytesRead = 0;
		bytesWritten = 0;

		totalSize = 0;
	}

	bool start()
	{
		if (!Thread2::start())
		{
			return false;
		}

		readerWorker();

		for(u64 i=0; buffers.size(); i++)
		{
			if (i > 5 * 100) // 5 seconds
			{
				error("timed out waiting for writer thread to complete\n");
				break;
			}
			nxSleep(10);
		}
		print("copy complete\n");
		return true;
	}

	virtual void readerWorker()
	{
		if (!readerInit())
		{
			return;
		}

		CopyBuffer* buffer = NULL;

		while (1)
		{
			buffer = buffers.peek();

			if (!(buffer->lock.acquireWriteLock()))
			{
				continue;
			}
			buffers.push();

			buffer->buffer().resize(0);

			if (!readChunk(bytesRead, buffer->buffer(), BUFFER_SIZE))
			{
				buffer->buffer().resize(0);
				buffer->lock.releaseWriteLock();
				break;
			}

			bytesRead += buffer->buffer().size();

			buffer->lock.releaseWriteLock();
		}

		readerExit();
	}

	virtual bool step() override
	{
		CopyBuffer* buffer = NULL;

		if (buffers.size())
		{
			if (!(buffer = buffers.first()))
			{
				return true;
			}

			if (!buffer->lock.acquireReadLock())
			{
				return true;
			}

			if (buffer->buffer().size())
			{
				writeChunk(bytesWritten, buffer->buffer());
				bytesWritten += buffer->buffer().size();
			}

			buffers.shift();
			buffer->lock.releaseReadLock();
		}
		else
		{
			nxSleep(1);
		}
		return true;
	}

	virtual bool init() override
	{
		if (!writerInit())
		{
			return false;
		}

		bytesWritten = 0;

		return true;
	}

	virtual void exit() override
	{
		writerExit();
	}

	virtual u64 writeChunk(u64 offset, const Buffer<u8>& buffer)
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

	virtual void readChunkComplete(u64 offset, const Buffer<u8>& buffer)
	{
		(void)offset;
		(void)buffer;
	}

	virtual void readChunkComplete(u64 offset, const void* buffer, u64 sz)
	{
		(void)offset;
		(void)buffer;
		(void)sz;
	}

	virtual bool readerInit()
	{
		return true;
	}

	virtual bool readerExit()
	{
		return true;
	}


	NcaId ncaId;

	u64 totalSize;
	u64 bytesRead;
	u64 bytesWritten;

	static const u32 BUFFER_SIZE = 0x800000;

	ChineseSdBuffer<CopyBuffer, 4> buffers;
};

class FileCopy : public Copy
{
public:
	FileCopy(string src, string dst) : Copy()
	{
		//fsrc = File::factory(src, "rb");
		fsrc = sptr<File>(new File());
		fsrc->open(src, "rb");

		//fdst = File::factory(dst, "wb");
		fdst = sptr<File>(new File());
		fdst->open(dst, "wb");
	}

	FileCopy(sptr<File> src, sptr<File> dst) : Copy()
	{
		fsrc = src;
		fdst = dst;
	}

	u64 writeChunk(u64 offset, const Buffer<u8>& buffer) override
	{
		fdst->write(buffer);
		return buffer.size();
	}

	u64 readChunk(u64 offset, Buffer<u8>& buffer, u64 sz) override
	{
		sz = fsrc->read(buffer, sz);
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

class FileStreamBuffer : Buffer<u8>
{
public:
	FileStreamBuffer(Copy* c) : Buffer<u8>()
	{
		m_parent = c;
		virtualSize = 0;
	}

	virtual bool resize(u64 newSize)
	{
		virtualSize = newSize;
		return true;
	}

	virtual u64 writeBuffer(const Buffer<u8>& v) override
	{
		return writeBuffer(v.buffer(), v.size());
	}

	virtual u64 writeBuffer(const void* p, u64 sz) override
	{
		if (sz >= BUFFER_SIZE)
		{
			flush();
			flush(p, sz);
		}
		else
		{
			cache.writeBuffer(p, sz);

			if (cache.size() >= BUFFER_SIZE)
			{
				flush();
			}
		}

		return sz;
	}

	virtual void setEOF() override
	{
		flush();
	}

	void flush()
	{
		if (cache.size())
		{
			flush(cache.buffer(), cache.size());
			cache.resize(0);
		}
	}

	void flush(const void* p, u64 sz)
	{
		if (m_parent)
		{
			m_parent->readChunkComplete(virtualSize, p, sz);
		}
	}

private:
	static const u32 BUFFER_SIZE = 0x80000;
	Copy* m_parent;
	u64 virtualSize;
	Buffer<u8> cache;
};

class FileStreamCopy : public FileCopy
{
public:
	FileStreamCopy(string src, string dst) : FileCopy(src, dst)
	{
	}

	FileStreamCopy(sptr<File> src, sptr<File> dst) : FileCopy(src, dst)
	{
	}

	void readChunkComplete(u64 offset, const Buffer<u8>& data) override
	{
		readChunkComplete(offset, data.buffer(), data.size());
	}

	void readChunkComplete(u64 offset, const void* data, u64 sz) override
	{
		(void)offset;

		while (!buffers.canWrite())
		{
			warning("read timeout\n");
		}

		CopyBuffer* buffer = buffers.peek();

		if (!(buffer->lock.acquireWriteLock()))
		{
			return;
		}

		buffers.push();

		buffer->buffer().set(data, sz);

		bytesRead += buffer->buffer().size();

		buffer->lock.releaseWriteLock();
	}

	void readerWorker() override
	{
		if (!readerInit())
		{
			return;
		}

		FileStreamBuffer f(this);

		fsrc->read((Buffer<u8>&)f);
		f.flush();

		readerExit();
	}
};