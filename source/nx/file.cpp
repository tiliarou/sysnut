#include "nut.h"
#include "log.h"
#include "nx/file.h"

File::File()
{
}

File::~File()
{
}

bool File::open(string& path, char* mode)
{
	if (isOpen())
	{
		warning("opening file with closing handle first %s\n", path);
		close();
	}

	f = fopen(path, mode);

	if (!f)
	{
		error("failed to open file %s\n", path);
		return false;
	}

	return true;
}

bool File::close()
{
	if (!f)
	{
		return false;
	}

	fclose(f);
	f = NULL;
	return true;
}

bool File::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}
	return fseek(f, offset, whence) == 0;
}

u64 File::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	return ftell(f);
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

u64 File::read(Buffer& buffer, u64 sz)
{
	if (!isOpen())
	{
		error("tried to read from closed file\n");
		return 0;
	}

	if (!sz)
	{
		sz = size();
	}

	buffer.resize(sz);

	return fread(buffer.c_str(), 1, sz, f);
}

bool File::isOpen()
{
	return f != NULL;
}