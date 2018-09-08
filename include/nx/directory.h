#pragma once
#include "nx/string.h"
#include "nx/buffer.h"
#include "nx/sptr.h"

class File;

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
	/*
	template<class K = File>
	sptr<K> open()
	{
		sptr<File>& f = open();
		return f;
	}
	*/

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
	virtual Array<sptr<FileEntry>>& files() { return m_files; };

	virtual const Array<sptr<Directory>>& directories() const { return m_directories; };
	virtual Array<sptr<Directory>>& directories() { return m_directories; };
protected:
	Array<sptr<FileEntry>> m_files;
	Array<sptr<Directory>> m_directories;
};