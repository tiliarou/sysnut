#pragma once
#include "nx/string.h"
#include "nx/buffer.h"
#include <stdio.h>

class File
{
public:
	File();
	virtual ~File();
	virtual bool open(string& path, char* mode = "rb");
	bool setPartition(File* parent, u64 offset, u64 sz);
	virtual bool close();
	bool seek(u64 offset, int whence = 0);
	bool rewind();
	u64 tell();
	u64 size();
	u64 read(Buffer& buffer, u64 sz = 0);

	u64& partitionOffset() { return m_partitionOffset; }
	u64& partitionSize() { return m_partitionSize; }

	bool isOpen();
	bool isPartition() { return m_parent != NULL; }

protected:
	bool setParent(File* parent);
	FILE* f = NULL;
	File* m_parent = NULL;
	u64 m_partitionOffset = 0;
	u64 m_partitionSize = 0;
	u64 m_size = 0;
};