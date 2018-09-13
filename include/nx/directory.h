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

	template<class T = File>
	sptr<T> open()
	{
		return sptr<T>(reinterpret_cast<T*>(open()));
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

	virtual File* open()
	{
		return NULL;
	}

	string m_name;
	u64 m_size;
};

class DirectoryFiles : public Array<sptr<FileEntry>>
{
public:
	DirectoryFiles() : Array<sptr<FileEntry>>()
	{
	}

	bool contains(const string fileName) const
	{
		for (auto& f : *this)
		{
			if (f->name() == fileName)
			{
				return true;
			}
		}

		return false;
	}

	sptr<Array<sptr<FileEntry>>> find(const char* ext)
	{
		string extension(ext);
		auto result = new Array<sptr<FileEntry>>();
		for (auto& f : *this)
		{
			if (f->name().endsWith(extension))
			{
				result->push(f);
			}
		}
		return sptr<Array<sptr<FileEntry>>>(result);
	}

private:
};

class Cnmt;
class ContentStorage;

class Directory
{
public:
	Directory();
	virtual ~Directory();

	//virtual const DirectoryFiles& files() const { return m_files; };
	virtual DirectoryFiles& files() { return m_files; };

	template<class T>
	sptr<Array<sptr<FileEntry>>> files2()
	{
		return files().where(
			[](sptr<FileEntry>& f) -> bool
			{
				string ext(mapTypeExtension<T>());
				return f->name().endsWith(ext);
			});
	}

	template<class T>
	sptr<T> openFile(string name)
	{
		for (auto& f : files())
		{
			if (f->name() == name)
			{
				return f->open<T>();
			}
		}
		return NULL;
	}

	virtual const Array<sptr<Directory>>& directories() const { return m_directories; };
	virtual Array<sptr<Directory>>& directories() { return m_directories; };

	string& dirPath() { return m_dirPath; }

	bool install();
protected:
	string m_dirPath;
	DirectoryFiles m_files;
	Array<sptr<Directory>> m_directories;
};
