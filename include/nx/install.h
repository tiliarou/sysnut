#pragma once
#include "nx/cnmt.h"
#include "nx/ipc/ns_ext.h"
#include "nx/contentstorage.h"

class cnmt;
class storage;
class dir;

class Install
{
public:
	Install(Directory* dir, Cnmt* cnmt);
	bool install();
	bool installContentMetaRecords(Buffer<u8>& installContentMetaBuf);
	bool installApplicationRecord();
private:
	Cnmt* cnmt;
	ContentStorage storage;
	Directory* dir;
};
