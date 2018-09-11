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

bool Directory::install(Cnmt* cnmt)
{
	Array<integer<128>> rightsIds;
	ContentStorage storage(FsStorageId_SdCard);
	print("installing cnmt\n");

	for (auto& content : *cnmt)
	{
		string ncaFile = hx(content.record.ncaId) + ".nca";
		print("content %x, %s\n", content.record.contentType, ncaFile.c_str());

		if (!files().contains(ncaFile))
		{
			error("could not find file! %s\n", ncaFile);
			return false;
		}

		//auto& nca = files().open<Nca>(ncaFile);
		Nca* nca;
		rightsIds.push(nca->rightsId());
	}

	// sanity check, make sure the cert and tik are available for all titleRights enabled NCA's slated to be installed
	for (auto& rightsId : rightsIds)
	{
		string ticketFile = hx(rightsId) + ".tik";
		string certFile = hx(rightsId) + ".cert";

		if (!files().contains(ticketFile))
		{
			error("could not find ticket! %s\n", ticketFile);
			return false;
		}

		if (!files().contains(certFile))
		{
			error("could not find cert! %s\n", ticketFile);
			return false;
		}
	}

	for (auto& content : *cnmt)
	{
		string ncaFile = hx(content.record.ncaId) + ".nca";

		//auto& nca = files().open<Nca>(ncaFile);
		Nca* nca;
		rightsIds.push(nca->rightsId());

		storage.deletePlaceholder(content.record.ncaId);
		//storage.createPlaceholder(content.record.ncaId, content.record.ncaId, content.record.size);

		// write stuff

		storage.reg(content.record.ncaId, content.record.ncaId);

		storage.deletePlaceholder(content.record.ncaId);
	}

	/*
	m_contentMeta.GetInstallContentMeta(installContentMetaBuf, cnmtContentRecord, m_ignoreReqFirmVersion);

	InstallContentMetaRecords(installContentMetaBuf);
	InstallApplicationRecord();

	InstallTicketCert();
	*/


	return true;
}

bool Directory::install()
{
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
					install(cnmt.get());
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