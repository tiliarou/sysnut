#include "nx/buffer.h"
#include <stdlib.h>
#include "log.h"
#include <string.h>

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
	if (buffer())
	{
		free(buffer());
		buffer() = NULL;
	}
}

bool Buffer::resize(u64 newSize)
{
	void* newBuffer = malloc(newSize);

	if (!newBuffer)
	{
		fatal("out of memory, tried to alloc %d bytes\n", newSize);
		return false;
	}

	if (buffer())
	{
		memcpy(newBuffer, buffer(), size());
		free(buffer());
	}

	buffer() = newBuffer;
	size() = newSize;
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