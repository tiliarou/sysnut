#include "nut.h"
#include "log.h"
#include "nx/file.h"
#include "nx/diskfile.h"

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

	DiskFile* f = new DiskFile();


	if (!f->open(path, mode))
	{
		error("failed to open file %s\n", path);
		delete f;
		return false;
	}

	setParent(f);

	if (!init())
	{
		delete f;
		return false;
	}

	this->path() = path;

	return true;
}

bool File::open2(File* f, u64 offset, u64 sz)
{
	if (!sz)
	{
		sz = f->size() - offset;
	}

	setParent(f);
	partitionOffset() = offset;
	partitionSize() = sz;

	print("created partition offset = %x, size = %x\n", offset, sz);
	return init();
}

u64 File::read(Buffer& buffer, u64 sz)
{
	if (!isOpen())
	{
		error("tried to read from closed file\n");
		return 0;
	}

	return m_parent->read(buffer, sz);
}

bool File::isOpen()
{
	return m_parent != NULL;
}

u64 File::size()
{
	if (!isOpen())
	{
		error("tried to size on closed file\n");
		return false;
	}

	if (isPartition())
	{
		return partitionSize();
	}

	return m_parent->size();
}

u64 File::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	u64 pos = parent()->tell();

	if (partitionOffset() > pos)
	{
		return 0;
	}

	return pos - partitionOffset();
}

bool File::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}

	switch (whence)
	{
		case SEEK_SET:
			return parent()->seek(offset + partitionOffset(), SEEK_SET);
		case SEEK_CUR:
			return parent()->seek(offset, SEEK_CUR);
		case SEEK_END:
			return parent()->seek(size() - partitionOffset() - offset, SEEK_SET);
	}

	return false;
}

bool File::rewind()
{
	return seek(0);
}

bool File::close()
{
	if (!isOpen())
	{
		return false;
	}

	m_parent->close();
	m_parent = NULL;

	return true;
}

bool File::init()
{
	if (!isOpen())
	{
		return false;
	}

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
