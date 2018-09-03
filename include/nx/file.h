#pragma once
#include "nx/string.h"
#include "nx/buffer.h"
#include <stdio.h>

class File
{
public:
	File();
	~File();
	bool open(string& path, char* mode = "rb");
	bool close();
	bool seek(u64 offset, int whence);
	u64 tell();
	u64 size();
	u64 read(Buffer& buffer, u64 sz = 0);

	bool isOpen();

private:
	FILE* f = NULL;
};