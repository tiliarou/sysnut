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

	for (auto& i : *cnmts)
	{
		Nca nca;
		if (nca.open2(i->open()))
		{
			for (const auto& pfs0 : nca.directories())
			{
				Cnmt cnmt;
				if (cnmt.open2(pfs0->files().first()->open()))
				{
					print("installing cnmt\n");
				}
				else
				{
					print("Failed to open cnmt!\n");
				}
			}
		}
		else
		{
			error("Could not open cnmt nca\n");
		}
	}
	return true;
}