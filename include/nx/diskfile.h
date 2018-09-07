#pragma once
#include "nx/file.h"
#include "nx/string.h"
#include "nx/buffer.h"
#include <stdio.h>

class DiskFile : public File
{
public:
	DiskFile();
	virtual ~DiskFile();
	virtual bool open(string& path, char* mode = "rb");
	virtual bool init();
	virtual bool close();
	bool seek(u64 offset, int whence = 0);
	bool rewind();
	u64 tell();
	u64 size();
	virtual u64 read(Buffer& buffer, u64 sz = 0);

	bool isOpen();
	string& path() { return m_path; }

protected:
	FILE* f = NULL;
	string m_path;
	u64 m_size = 0;
};