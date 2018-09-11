#include "nx/directory.h"
#include "nx/cnmt.h"
#include "nx/ipc/ns_ext.h"
#include "nx/contentstorage.h"

Directory::Directory()
{
}

Directory::~Directory()
{
}

bool Directory::installApplicationRecord(const ContentStorage* storage, const Cnmt* cnmt)
{
	TitleId baseId = cnmt->contentMetaHeader()->titleId.baseId();
	Result rc = 0;
	Buffer<ContentStorageRecord> storageRecords;
	u32 contentMetaCount = 0;

	// 0x410: The record doesn't already exist

	if ((rc = nsCountApplicationContentMeta(baseId, &contentMetaCount)) && rc != 0x410)
	{
		error("Failed to count application content meta\n");
		return false;
	}

	rc = 0;

	if (contentMetaCount > 0)
	{
		storageRecords.resize(contentMetaCount);

		size_t contentStorageBufSize = contentMetaCount * sizeof(ContentStorageRecord);
		u32 entriesRead;

		if (nsListApplicationRecordContentMeta(0, baseId, storageRecords.buffer(), storageRecords.sizeBytes(), &entriesRead))
		{
			error("Failed to list application record content meta\n");
			return false;
		}

		if (entriesRead != storageRecords.size())
		{
			error("Mismatch between entries read and content meta count\n");
			return false;
		}
	}

	ContentStorageRecord storageRecord;
	storageRecord.metaRecord = cnmt->contentMetaKey();
	storageRecord.storageId = storage->storageId();

	storageRecords.push(storageRecord);


	nsDeleteApplicationRecord(baseId);

	if (nsPushApplicationRecord(baseId, 0x3, storageRecords.buffer(), storageRecords.sizeBytes()))
	{
		error("Failed to push application record\n");
		return false;
	}

	return true;
}

bool Directory::installContentMetaRecords(const Cnmt* cnmt, Buffer<u8>& installContentMetaBuf)
{
	NcmContentMetaDatabase contentMetaDatabase;
	NcmMetaRecord contentMetaKey = cnmt->contentMetaKey();

#ifndef _MSC_VER
	if (ncmOpenContentMetaDatabase(m_destStorageId, &contentMetaDatabase))
	{
		error"Failed to open content meta database\n");
		return false;
	}

	if (ncmContentMetaDatabaseSet(&contentMetaDatabase, &contentMetaKey, installContentMetaBuf.GetSize(), (NcmContentMetaRecordsHeader*)installContentMetaBuf.GetData()))
	{
		error("Failed to set content records\n");
		serviceClose(&contentMetaDatabase.s);
		return false;
	}

	if (ncmContentMetaDatabaseCommit(&contentMetaDatabase))
	{
		error("Failed to commit content records");
		serviceClose(&contentMetaDatabase.s);
		return false;
	}
#endif
	return true;

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

	if (!installContentMetaRecords(cnmt, cnmt->ncmContentMeta()))
	{
		error("Failed to install content meta records!\n");
		return false;
	}

	//InstallApplicationRecord();

	//InstallTicketCert();

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