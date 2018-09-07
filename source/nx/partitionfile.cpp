#include "nx/partitionfile.h"
#include "log.h"

PartitionFile::PartitionFile() : File()
{
}

PartitionFile::PartitionFile(File* parent, u64 offset, u64 sz) : File()
{
	if (!sz)
	{
		print("setting part size... %x %x\n", offset, parent->size());
		sz = parent->size() - offset;
	}
	else
	{
		print("parition %x, %x\n", (u32)offset, (u32)sz);
	}

	setParent(parent);
	partitionOffset() = offset;
	partitionSize() = sz;

	print("created partition %x, %x\n", offset, sz);
}

PartitionFile::~PartitionFile()
{
}

u64 PartitionFile::read(Buffer& buffer, u64 sz)
{
	if (!isOpen())
	{
		error("tried to read from closed file\n");
		return 0;
	}

	return m_parent->read(buffer, sz);
}

u64 PartitionFile::size()
{
	return partitionSize();
}

u64 PartitionFile::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	u64 pos = m_parent->tell();

	if (partitionOffset() > pos)
	{
		return 0;
	}

	return pos - partitionOffset();
}

bool PartitionFile::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}

	return m_parent->seek(partitionOffset() + offset, whence);
}

bool PartitionFile::close()
{
	if (!isOpen())
	{
		return false;
	}

	m_parent->close();
	m_parent = NULL;

	return true;
}

bool PartitionFile::init()
{
	if (!isOpen())
	{
		return false;
	}

	return m_parent->init();
}
