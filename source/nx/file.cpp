#include "nut.h"
#include "nx/file.h"

File::File()
{
}

File::~File()
{
}

bool File::open(string path)
{
	return true;
}

bool File::seek(u64 offset, int whence)
{
	return true;
}

u64 File::tell()
{
	return 0;
}

u64 File::size()
{
	u64 currentPosition = tell();

	if (!seek(0, SEEK_END))
	{
		return 0;
	}

	u64 sz = tell();

	seek(currentPosition, SEEK_SET);

	return sz;
}