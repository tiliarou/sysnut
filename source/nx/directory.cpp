#include "nx/directory.h"
#include "nx/cnmt.h"

Directory::Directory()
{
}

Directory::~Directory()
{
}

bool Directory::install()
{
	auto& cnmts = files().where([](sptr<FileEntry>& f) -> bool {return f->name().endsWith(string(".cnmt.nca")); });

	for (auto& f : *cnmts)
	{
		Cnmt cnmt;
		if (cnmt.open2(f->open()))
		{
			print("hmm\n");
		}
	}
	return true;
}