#include "nx/install.h"
#include "nx/directory.h"
#include "nx/ipc/es.h"

Install::Install(Directory* dir, Nca* cnmtNca, Cnmt* cnmt)
{
	this->dir = dir;
	this->cnmt = cnmt;
	this->cnmtNca = cnmtNca;
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

bool Install::installContentMetaRecords(Buffer<u8> installContentMetaBuf)
{
	NcmContentMetaDatabase contentMetaDatabase;
	NcmMetaRecord contentMetaKey = cnmt->contentMetaKey();

#ifndef _MSC_VER
	if (ncmOpenContentMetaDatabase(storage.storageId(), &contentMetaDatabase))
	{
		error("Failed to open content meta database\n");
		return false;
	}

	if (ncmContentMetaDatabaseSet(&contentMetaDatabase, &contentMetaKey, installContentMetaBuf.size(), (NcmContentMetaRecordsHeader*)installContentMetaBuf.buffer()))
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
#else
	FILE* f = fopen("installContentMeta.bin", "wb+");
	if (f)
	{
		fwrite(installContentMetaBuf.buffer(), 1, installContentMetaBuf.size(), f);
		fclose(f);
	}
#endif
	return true;

}

bool Install::installNca(File* nca, NcaId ncaId)
{
	Buffer<u8> buffer;
	const u64 chunkSize = 0x100000;
	string ncaFile = hx(ncaId) + ".nca";

	if (storage.has(ncaId))
	{
		print("already installed, skipping %s\n", ncaFile.c_str());
		return true;
	}

	storage.deletePlaceholder(ncaId);
	storage.createPlaceholder(ncaId, ncaId, nca->size());

	print("writing %s ", nca->path().c_str());
	u64 i = 0;

	nca->rewind();

	while (nca->read(buffer, chunkSize))
	{
		print(".");
		storage.writePlaceholder(ncaId, i, buffer.buffer(), buffer.size());
		i += buffer.size();
	}
	print("fin\n");

	if (!storage.reg(ncaId, ncaId))
	{
		error("Failed to register %s\n", ncaFile.c_str());
		return false;
	}

#ifndef _MSC_VER
	storage.deletePlaceholder(ncaId);
#endif

	return true;
}

bool Install::install()
{
	Array<RightsId> rightsIds;
	print("installing cnmt\n");

	for (auto& content : *cnmt)
	{
		string ncaFile = hx(content.record.ncaId) + ".nca";

		if (!dir->files().contains(ncaFile))
		{
			error("could not find file! %s\n", ncaFile);
			return false;
		}

		auto nca = dir->openFile<Nca>(ncaFile);
		if (nca->rightsId().titleId() || nca->rightsId().masterKeyRev())
		{
			rightsIds.push(nca->rightsId());
		}
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

		auto nca = dir->openFile<File>(ncaFile);

		if (!installNca(nca.get(), content.record.ncaId))
		{
			return false;
		}

	}

	auto hash = uhx(cnmtNca->path());

	if (!installNca((File*)cnmtNca, *reinterpret_cast<integer<128>*>(hash.buffer())))
	{
		error("Failed to install cnmt!\n");
	}

	if (!installContentMetaRecords(cnmt->ncmContentMeta()))
	{
		error("Failed to install content meta records!\n");
		return false;
	}

	if (!installApplicationRecord())
	{
		error("Failed to install application record\n");
		return false;
	}

	for (auto& rightsId : rightsIds)
	{
		string ticketFile = hx(rightsId) + ".tik";
		string certFile = hx(rightsId) + ".cert";

		auto ticket = dir->openFile<File>(ticketFile);
		auto cert = dir->openFile<File>(certFile);

		Buffer<u8> ticketBuffer, certBuffer;
		ticket->read(ticketBuffer);
		cert->read(certBuffer);

		if (esImportTicket(ticketBuffer.buffer(), ticketBuffer.size(), certBuffer.buffer(), certBuffer.size()))
		{
			error("Failed to import ticket %s\n", ticketFile.c_str());
			return false;
		}
		else
		{
			print("Imported %s\n", ticketFile.c_str());
		}
	}

	return true;
}

string getBaseTitleName(TitleId baseTitleId)
{
	Result rc = 0;
	NsApplicationControlData appControlData;
	size_t sizeRead;

	if (R_FAILED(rc = nsGetApplicationControlData(0x1, swapEndian(baseTitleId), &appControlData, sizeof(NsApplicationControlData), &sizeRead)))
	{
		error("Failed to get application control data. Error code: 0x%08x\n", rc);
		return hx(baseTitleId);
	}

	if (sizeRead < sizeof(appControlData.nacp))
	{
		error("Incorrect size for nacp\n");
		return "Unknown2";
	}

	NacpLanguageEntry *languageEntry;

	if (R_FAILED(rc = nacpGetLanguageEntry(&appControlData.nacp, &languageEntry)))
	{
		error("Failed to get language entry. Error code: 0x%08x\n", rc);
		return hx(baseTitleId);
	}

	if (languageEntry == NULL)
	{
		error("Language entry is null! Error code: 0x%08x\n", rc);
		return hx(baseTitleId);
	}

	return languageEntry->name;
}