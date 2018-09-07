#include "nut.h"
#include "log.h"
#include "nx/file.h"

File::File()
{
}

File::~File()
{
	close();
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

	size(); // just cache the file size

	if (!init())
	{
		return false;
	}

	return true;
}

bool File::init()
{
	return true;
}

bool File::setParent(File* parent)
{
	if (isOpen())
	{
		close();
	}

	// todo notify parent
	m_parent = parent;
	return true;
}

bool File::setPartition(File* parent, u64 offset, u64 sz)
{
	setParent(parent);
	partitionOffset() = offset;
	partitionSize() = sz;

	return init();
}

bool File::close()
{
	if (!f)
	{
		return false;
	}

	if (f)
	{
		fclose(f);
		f = NULL;
	}

	if (m_parent)
	{
		// todo notify parent
		m_parent = NULL;
	}

	return true;
}

bool File::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}
	return fseek(f, partitionOffset() + offset, whence) == 0;
}

bool File::rewind()
{
	return seek(0);
}

u64 File::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	u64 pos = ftell(f);

	if (partitionOffset() > pos)
	{
		return 0;
	}

	return pos - partitionOffset();
}

u64 File::size()
{
	if (isPartition())
	{
		return partitionSize();
	}

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

	if (f)
	{
		return fread(buffer.c_str(), 1, sz, f);
	}

	if (m_parent)
	{
		return m_parent->read(buffer, sz);
	}
}

bool File::isOpen()
{
	return f != NULL || m_parent != NULL;
}
