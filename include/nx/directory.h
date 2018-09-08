#pragma once
#include "nx/string.h"

#define MAX_FILE_ENTRIES 0x10

class FileEntry
{
public:
protected:
	string m_name;
	u64 m_size;
};

class Directory
{
public:
	Directory();
	virtual ~Directory();
private:
};