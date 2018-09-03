#pragma once
#include "nx/string.h"

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

class File
{
public:
	File();
	~File();
	bool open(string path);
	bool seek(u64 offset, int whence);
	u64 tell();
	u64 size();

private:
};