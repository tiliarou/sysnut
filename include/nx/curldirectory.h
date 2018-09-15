#pragma once

#include "nx/directory.h"
#include "nx/url.h"

class CurlDirectory : public Directory
{
public:
	CurlDirectory(string url);
	virtual DirectoryFiles& files() override;

	virtual string resolvePath(FileEntry& f);
private:
};