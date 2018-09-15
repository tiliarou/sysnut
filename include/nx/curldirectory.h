#pragma once

#include "nx/directory.h"

class CurlDirectory : public Directory
{
public:
	CurlDirectory(string url);
	virtual DirectoryFiles& files() override;
private:
};