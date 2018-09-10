#include "nx/directory.h"
#include "nx/cnmt.h"
#include "nx/contentstorage.h"

Directory::Directory()
{
}

Directory::~Directory()
{
}

//auto& cnmts = files().where([](sptr<FileEntry>& f) -> bool {return f->name().endsWith(string(".cnmt.nca")); });

bool Directory::install()
{
	ContentStorage storage(FsStorageId_SdCard);

	auto& cnmts = files().find(".cnmt.nca");

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
						string ncaFile = hx(content.record.ncaId) + ".nca";
						print("content %x, %s\n", content.record.contentType, ncaFile.c_str());

						if (!files().contains(ncaFile))
						{
							error("could not find file! %s\n", ncaFile);
							goto skip;
						}

						storage.deletePlaceholder(content.record.ncaId);
						//storage.createPlaceholder(content.record.ncaId, content.record.ncaId, content.record.size);

						// write stuff

						storage.reg(content.record.ncaId, content.record.ncaId);

						storage.deletePlaceholder(content.record.ncaId);
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
	skip:;
	}
	return true;
}