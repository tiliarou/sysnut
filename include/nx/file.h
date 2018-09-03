#pragma once
#include "nx/string.h"

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