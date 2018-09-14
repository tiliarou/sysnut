#pragma once
#include "nx/cnmt.h"
#include "nx/ipc/ns_ext.h"
#include "nx/contentstorage.h"

class cnmt;
class storage;
class dir;
class Nca;

string getBaseTitleName(TitleId baseTitleId);

class Install
{
public:
	Install(Directory* dir, Nca* cnmtNca, Cnmt* cnmt);
	bool install();
	bool installNca(File* nca, NcaId ncaId);
	bool installContentMetaRecords(Buffer<u8> installContentMetaBuf);
	bool installApplicationRecord();
private:
	Nca* cnmtNca;
	Cnmt* cnmt;
	ContentStorage storage;
	Directory* dir;
};
