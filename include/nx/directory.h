#pragma once
#include "nx/string.h"
#include "nx/buffer.h"
#include "nx/sptr.h"

class File;

#define MAX_FILE_ENTRIES 0x10

class FileEntry
{
public:
	FileEntry()
	{
		m_size = 0;
	}

	FileEntry(string fileName, u64 sz)
	{
		name() = fileName;
		size() = sz;
	}

	virtual ~FileEntry()
	{
	}

	virtual sptr<File> open()
	{
		return NULL;
	}

	const string& name() const { return m_name; }
	string& name() { return m_name; }

	const u64& size() const { return m_size; }
	u64& size() { return m_size; }
protected:
	string m_name;
	u64 m_size;
};

class Directory
{
public:
	Directory();
	virtual ~Directory();

	virtual const Array<sptr<FileEntry>>& files() const { return m_files; };
protected:
	virtual Array<sptr<FileEntry>>& files() { return m_files; };
	Array<sptr<FileEntry>> m_files;
};