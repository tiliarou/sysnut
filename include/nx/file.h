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

protected:
	string m_path;
	File* m_parent = NULL;
};