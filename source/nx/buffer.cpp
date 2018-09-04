#include "nx/buffer.h"
#include <stdlib.h>
#include "log.h"
#include <string.h>

Buffer::Buffer()
{
	buffer() = NULL;
	size() = 0;
	bufferSize() = 0;
}

Buffer::~Buffer()
{
	close();
}

Buffer::Buffer(const Buffer& src)
{
	resize(src.size());
	set(src.buffer(), src.size());
}

Buffer& Buffer::operator=(const Buffer& src)
{
	resize(src.size());
	set(src.buffer(), src.size());
	return *this;
}

bool Buffer::resize(u64 newSize)
{
	if (buffer() && newSize <= bufferSize())
	{
		size() = newSize;
		return true;
	}

	u64 newBufferSize = (newSize / BUFFER_ALIGN) * BUFFER_ALIGN + BUFFER_ALIGN;

	void* newBuffer = malloc((size_t)newBufferSize);

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

char* Buffer::c_str() const
{
	if (!buffer())
	{
		return "";
	}

	return (char*)buffer();
}

bool Buffer::close()
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

bool Buffer::set(const void* src, u64 sz)
{
	if (!resize(sz))
	{
		return false;
	}
	memcpy(buffer(), src, (size_t)sz);
	return true;
}