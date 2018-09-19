#pragma once

#include "nx/directory.h"
#include "nx/url.h"

class CurlDirectory : public Directory
{
public:
	CurlDirectory(Url url);
	virtual DirectoryFiles& files() override;

	virtual Url resolvePath(const FileEntry* f);
private:
};