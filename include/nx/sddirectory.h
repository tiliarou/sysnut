#pragma once
#include "nx/directory.h"

class SdDirectory : public Directory
{
public:
	SdDirectory(string path);
	virtual DirectoryFiles& files() override;
private:
};