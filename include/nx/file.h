#pragma once
#include "nx/string.h"
#include "nx/buffer.h"
#include "nx/sptr.h"
#include "nx/integer.h"
#include "nx/url.h"
#include <stdio.h>

#define MAX_FILE_CHILDREN 0x10

class File
{
public:
	File();
	virtual ~File();

	virtual bool open(Url path, const char* mode = "rb");
	bool open2(sptr<File>& f, u64 offset = 0, u64 sz = 0);

	static bool copy(string src, string dst);

	static sptr<File> factory(Url& path, const char* mode = "rb");
	static sptr<File> factory(Url& path, sptr<File>& f, u64 offset = 0, u64 sz = 0);

	static File* factoryRawPtr(Url& path, const char* mode = "rb");
	static File* factoryRawPtr(Url& path, sptr<File>& f, u64 offset = 0, u64 sz = 0);

	integer<256> sha256();

	template<class T = File>
	static sptr<T> factory(Url& path, char* mode = "rb")
	{
		return reinterpret_cast<T*>(factoryRawPtr(path, mode));
	}

	template<class T = File>
	static sptr<T> factory(Url& path, sptr<File>& f, u64 offset = 0, u64 sz = 0)
	{
		return reinterpret_cast<T*>(factoryRawPtr(path, f, offset, sz));
	}

	virtual bool init();
	virtual bool close();
	virtual bool seek(u64 offset, int whence = 0);
	bool rewind();
	virtual u64 tell();
	virtual u64 size();
	virtual u64 read(Buffer<u8>& buffer, u64 sz = 0);

	bool isOpen();
	Url& path() { return m_path; }

	sptr<File>& parent() { return m_parent; }

	bool setParent(sptr<File> parent);
	bool isPartition() { return partitionSize() != 0; }

	u64& partitionOffset() { return m_partitionOffset; }
	u64& partitionSize() { return m_partitionSize; }

	virtual bool registerChild(File* child);
	virtual bool unregisterChild(File* child);
	virtual void onChildless();
	u64 childrenCount();

	sptr<File>& ptr() { return m_ptr; }

protected:

	Url m_path;
	sptr<File> m_parent;
	u64 m_partitionOffset = 0;
	u64 m_partitionSize = 0;
	File* m_children[MAX_FILE_CHILDREN];

	sptr<File> m_ptr;
};



