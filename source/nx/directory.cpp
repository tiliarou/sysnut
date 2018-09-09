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
		auto nca = i->open<Nca>();

		if (nca)
		{
			if (nca->directories().size() && nca->directories().first()->files().size())
			{
				// cnmt nca's contain a single partition with a single file
				auto cnmt = nca->directories().first()->files().first()->open<Cnmt>();
				if (cnmt)
				{
					print("installing cnmt\n");

					for (auto& content : *cnmt)
					{
						print("content %x, %s\n", content.record.contentType, hx(content.record.ncaId).c_str());
					}
				}
				else
				{
					error("Failed to open cnmt!\n");
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