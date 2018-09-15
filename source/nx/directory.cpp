#include "nx/directory.h"
#include "nx/cnmt.h"
#include "nx/ipc/ns_ext.h"
#include "nx/contentstorage.h"
#include "nx/install.h"
#include "nx/curldirectory.h"
#include "nx/sddirectory.h"

#include <curl/curl.h>

File* FileEntry::open()
{
	string path = parent()->resolvePath(*this);
	return File::factoryRawPtr(path);
}

sptr<Directory> Directory::openDir(Url url)
{
	Directory* p = NULL;
	if (url.scheme() == "ftp" || url.scheme() == "ftps" || url.scheme() == "sftp" || url.scheme() == "tftp" || url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "scp")
	{
		p = new CurlDirectory(url);
	}
	else
	{
		p = new SdDirectory(url);
	}

	return sptr<Directory>(p);
}

Directory::Directory()
{
}

Directory::~Directory()
{
}


bool Directory::install()
{
	auto cnmts = files().find(".cnmt.nca");

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
					Install install(this, nca.get(), cnmt.get());
					install.install();
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

string Directory::resolvePath(FileEntry& f)
{
	return dirPath().str() + f.name();
}