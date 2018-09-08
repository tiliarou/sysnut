#pragma once
#include "nut.h"
#include "log.h"

#define BUFFER_ALIGN 0xFF

template<class T=u8>
class Buffer
{
public:
	Buffer()
	{
		buffer() = NULL;
		size() = 0;
		bufferSize() = 0;
	}

	~Buffer()
	{
		close();
	}

	Buffer(const Buffer<T>& src)
	{
		resize(src.size());
		set(src.buffer(), src.size());
	}

	Buffer<T>& operator=(const Buffer<T>& src)
	{
		resize(src.size());
		set(src.buffer(), src.size());
		return *this;
	}

	bool slice(Buffer<T>& out, s64 start, s64 end) const
	{
		//print("slicing %d, %d\n", (s32)start, (s32)end);
		u64 sz = end - start;

		if (!out.resize(sz))
		{
			return false;
		}

		memcpy(out.buffer(), c_str() + start, (size_t)sz);

		return true;
	}

	bool resize(u64 newSize)
	{
		if (buffer() && newSize <= bufferSize())
		{
			size() = newSize;
			return true;
		}

		u64 newBufferSize = (newSize / BUFFER_ALIGN) * BUFFER_ALIGN + BUFFER_ALIGN;

		T* newBuffer = (T*)malloc((size_t)newBufferSize);

		if (!newBuffer)
		{
			fatal("out of memory, tried to alloc %d bytes\n", newSize);
			return false;
		}

		if (buffer())
		{
			memcpy(newBuffer, buffer(), (size_t)size());
			free(buffer());
		}

		buffer() = newBuffer;
		size() = newSize;
		bufferSize() = newBufferSize;
		return true;
	}

	const T* buffer() const { return m_buffer; }
	T*& buffer() { return m_buffer; }

	const u64 size() const { return m_size; }
	u64& size() { return m_size;  }

	const u64& bufferSize() const { return m_bufferSize; }
	u64& bufferSize() { return m_bufferSize; }

	bool set(const void* src, u64 sz)
	{
		if (!resize(sz))
		{
			return false;
		}
		memcpy(buffer(), src, (size_t)sz);
		return true;
	}

	bool close()
	{
		if (!buffer())
		{
			return false;
		}

		free(buffer());
		size() = 0;
		bufferSize() = 0;
		buffer() = NULL;
		return true;
	}

	char* c_str(u64 i = 0) const
	{
		if (!buffer())
		{
			return "";
		}

		return (char*)buffer() + i;
	}

	void dump(int sz = 0, int offset = 0) const
	{
		if (!sz)
		{
			sz = size() - offset;
		}
		else
		{
			sz += offset;
		}

		for (unsigned long i = offset; i < sz; i++)
		{
			if ((i + 1) % 32 == 0)
			{
				debug("%2.2X\n", (u8)c_str()[i]);
			}
			else if ((i + 1) % 4 == 0)
			{
				debug("%2.2X ", (u8)c_str()[i]);
			}
			else
			{
				debug("%2.2X", (u8)c_str()[i]);
			}
		}
		debug("\n");
	}

protected:

	T* m_buffer = NULL;
	u64 m_size = 0;
	u64 m_bufferSize = 0;
};
