#include "nx/install.h"
#include "nx/directory.h"

Install::Install(Directory* dir, Cnmt* cnmt)
{
	this->dir = dir;
	this->cnmt = cnmt;
}

bool Install::installApplicationRecord()
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
	storageRecord.storageId = storage.storageId();

	storageRecords.push(storageRecord);


	nsDeleteApplicationRecord(baseId);

	if (nsPushApplicationRecord(baseId, 0x3, storageRecords.buffer(), storageRecords.sizeBytes()))
	{
		error("Failed to push application record\n");
		return false;
	}

	return true;
}

bool Install::installContentMetaRecords(Buffer<u8>& installContentMetaBuf)
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

bool Install::install()
{
	Array<integer<128>> rightsIds;
	ContentStorage storage(FsStorageId_SdCard);
	print("installing cnmt\n");

	for (auto& content : *cnmt)
	{
		string ncaFile = hx(content.record.ncaId) + ".nca";
		print("content %x, %s\n", content.record.contentType, ncaFile.c_str());

		if (!dir->files().contains(ncaFile))
		{
			error("could not find file! %s\n", ncaFile);
			return false;
		}

		//auto& nca = files().open<Nca>(ncaFile);
		//rightsIds.push(nca->rightsId());
	}

	for (auto& rightsId : rightsIds)
	{
		string ticketFile = hx(rightsId) + ".tik";
		string certFile = hx(rightsId) + ".cert";

		if (!dir->files().contains(ticketFile))
		{
			error("could not find ticket! %s\n", ticketFile);
			return false;
		}

		if (!dir->files().contains(certFile))
		{
			error("could not find cert! %s\n", ticketFile);
			return false;
		}
	}

	for (auto& content : *cnmt)
	{
		string ncaFile = hx(content.record.ncaId) + ".nca";

		//auto& nca = files().open<Nca>(ncaFile);
		//rightsIds.push(nca->rightsId());

		storage.deletePlaceholder(content.record.ncaId);
		//storage.createPlaceholder(content.record.ncaId, content.record.ncaId, content.record.size);

		// write stuff

		storage.reg(content.record.ncaId, content.record.ncaId);

		storage.deletePlaceholder(content.record.ncaId);
	}

	if (!installContentMetaRecords(cnmt->ncmContentMeta()))
	{
		error("Failed to install content meta records!\n");
		return false;
	}

	installApplicationRecord();

	//InstallTicketCert();

	return true;
}