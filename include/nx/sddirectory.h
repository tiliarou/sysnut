#pragma once
#include "nx/directory.h"

class SdDirectory : public Directory
{
public:
	SdDirectory(Url path);
	virtual DirectoryFiles& files() override;
private:
};