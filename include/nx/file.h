#pragma once
#include "nx/string.h"
#include "nx/buffer.h"
#include <stdio.h>

class File
{
public:
	File();
	virtual ~File();
	bool open(string& path, char* mode = "rb");
	bool open2(File* f, u64 offset, u64 sz);
	virtual bool init();
	virtual bool close();
	virtual bool seek(u64 offset, int whence = 0);
	bool rewind();
	virtual u64 tell();
	virtual u64 size();
	virtual u64 read(Buffer& buffer, u64 sz = 0);

	bool isOpen();
	string& path() { return m_path; }

	File*& parent() { return m_parent; }

	bool setParent(File* parent);
	bool isPartition() { return partitionSize() != 0; }

	u64& partitionOffset() { return m_partitionOffset; }
	u64& partitionSize() { return m_partitionSize; }

protected:
	string m_path;
	File* m_parent = NULL;
	u64 m_partitionOffset = 0;
	u64 m_partitionSize = 0;
};