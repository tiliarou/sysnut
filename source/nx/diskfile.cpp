#include "nut.h"
#include "log.h"
#include "nx/diskfile.h"

DiskFile::DiskFile()
{
}

DiskFile::~DiskFile()
{
	close();
}

bool DiskFile::open(Url path, const char* mode)
{
	if (isOpen())
	{
		warning("opening file with closing handle first %s\n", path.c_str());
		close();
	}

	f = fopen(path.c_str(), mode);

	if (!f)
	{
		error("failed to open file %s\n", path.c_str());
		return false;
	}

	size(); // just cache the file size

	this->path() = path;

	return true;
}

bool DiskFile::init()
{
	return true;
}

bool DiskFile::close()
{
	if (!f)
	{
		return false;
	}


	fclose(f);
	f = NULL;
	return true;
}

bool DiskFile::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}

	return fseek(f, (long)offset, whence) == 0;
}

bool DiskFile::rewind()
{
	return seek(0);
}

u64 DiskFile::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	u64 pos = ftell(f);

	return pos;
}

u64 DiskFile::size()
{
	if (m_size)
	{
		return m_size;
	}

	u64 currentPosition = tell();

	if (!seek(0, SEEK_END))
	{
		return 0;
	}

	m_size = tell();

	seek(currentPosition, SEEK_SET);

	return m_size;
}

u64 DiskFile::read(Buffer<u8>& buffer, u64 sz)
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

	u64 bytesRead = (u64)fread(buffer.buffer(), 1, (size_t)sz, f);
	buffer.resize(bytesRead);

	return bytesRead;

}

u64 DiskFile::write(const Buffer<u8>& buffer)
{
	if (!isOpen())
	{
		error("tried to write to closed file\n");
		return 0;
	}

	if (!buffer.size())
	{
		return 0;
	}

	return (u64)fwrite(buffer.buffer(), 1, buffer.size(), f);
}

bool DiskFile::isOpen()
{
	return f != NULL;
}

void DiskFile::onChildless()
{
	close();
}

