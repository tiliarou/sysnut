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

void Install::writerThread(InstallThreadContext* ctx)
{
	if (!ctx || !ctx->install)
	{
		return;
	}

	InstallBuffer* buffer = NULL;
	u64 offset = 0;
	ctx->i = 0;
	ctx->threadInit = true;
	ctx->threadRunning = true;

	while (ctx->threadRunning)
	{
		if (ctx->buffers.size())
		{
			if (!(buffer = ctx->buffers.first()))
			{
				continue;
			}

			if (!buffer->lock.acquireReadLock())
			{
				continue;
			}

			if (buffer->buffer().size())
			{
				ctx->install->storage.writePlaceholder(ctx->ncaId, offset, buffer->buffer().buffer(), buffer->buffer().size());
				offset += buffer->buffer().size();
			}

			//print("\rwriting %d%% %s ", int(totalSize ? (offset * 100 / totalSize) : 100), nca->path().c_str());

			ctx->buffers.shift();
			buffer->lock.releaseReadLock();
			ctx->i++;
		}
		else
		{
			nxSleep(100);
		}
	}
	ctx->threadRunning = false;
}

static inline int getNextCpuId()
{
#ifdef __SWITCH__
	const auto currProcNum = svcGetCurrentProcessorNumber();
	const auto nextProcNum = (currProcNum + 1) % 4;
	if (nextProcNum == 3) //ew, kernel cpu
		return 0;
	else
		return (int)nextProcNum;
#else
	return 0;
#endif
}

bool Install::installNca(File* nca, NcaId ncaId)
{
	if (!nca)
	{
		error("Attempted to install NULL NCA Pointer\n");
		return false;
	}

	const u64 chunkSize = 0x100000;
	string ncaFile = hx(ncaId) + ".nca";
	u64 totalSize = nca->size();

	if (storage.has(ncaId))
	{
		print("already installed, skipping %s\n", ncaFile.c_str());
		return true;
	}

	storage.deletePlaceholder(ncaId);
	storage.createPlaceholder(ncaId, ncaId, totalSize);

	print("writing 0%% %s ", nca->path().c_str());
	u64 i = 0;


	nca->rewind();

	//if (totalSize < 0x100000)
	{
		Buffer<u8> buffer;

		while (nca->read(buffer, chunkSize))
		{
			storage.writePlaceholder(ncaId, i, buffer.buffer(), buffer.size());
			i += buffer.size();

			print("\rwriting %d%% %s ", int(totalSize ? (i * 100 / totalSize) : 100), nca->path().c_str());
		}
	}
	/*else
	{
		u64 bufferIndex = 0;
		InstallThreadContext ctx(this, nca, ncaId);

#ifdef __SWITCH__
		memset(&ctx.t, 0, sizeof(ctx.t));
		if (threadCreate(&ctx.t, (void(*)(void*))&writerThread, &ctx, 1 * 1024 * 1024, 0x2D, getNextCpuId()))
		{
			error("Failed to create download thread!\n");
			return false;
		}

		if (threadStart(&ctx.t))
		{
			error("Failed to start download thread!\n");
			return false;
		}
#endif

		const u64 chunkSize = 0x100000 * 16;
		InstallBuffer* buffer = NULL;

		while (1)
		{
			buffer = ctx.buffers.peek();
			buffer->lock.acquireWriteLock();
			ctx.buffers.push();

			print("\rreading block %d\n", bufferIndex);

			buffer->buffer().resize(0);

			if (!nca->read(buffer->buffer(), chunkSize))
			{
				buffer->buffer().resize(0);
				buffer->lock.releaseWriteLock();
				break;
			}

			buffer->lock.releaseWriteLock();
			bufferIndex++;
		}
		print("exiting read loop.. waiting for buffers to empty.\n");

		for (int i = 0; i < 1000 && ctx.buffers.size(); i++)
		{
			if (i > 900)
			{
				error("Timed out waiting for buffers to flush\n");
				break;
			}
			nxSleep(10);
		}
		error("buffers flushed!\n");
	}
	*/
#ifdef __SWITCH__
	print("\n");
#endif

	/*if (!storage.reg(ncaId, ncaId))
	{
		error("Failed to register %s\n", ncaFile.c_str());
		return false;
	}*/

#ifndef _MSC_VER
	storage.deletePlaceholder(ncaId);
#endif

	return true;
}

bool Install::install()
{
	try
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

			if (!nca)
			{
				error("Failed to open %s\n", ncaFile.c_str());
				return false;
			}

			if (!installNca(nca.get(), content.record.ncaId))
			{
				error("Install failed\n");
				return false;
			}
		}

		auto hash = uhx(cnmtNca->path().baseName());

		if (!installNca((File*)cnmtNca, *reinterpret_cast<integer<128>*>(hash.buffer())))
		{
			error("Failed to install cnmt!\n");
			return false;
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

			ticket->rewind();
			ticket->read(ticketBuffer);

			cert->rewind();
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
	catch (...)
	{
		error("An unknown error occurred during installation\n");
		return false;
	}
}

string getBaseTitleName(TitleId baseTitleId)
{
#ifndef _MSC_VER
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
#else
	return "UNKNOWN";
#endif
}

void nxSleep(u64 ms)
{
#ifdef __SWITCH__
	svcSleepThread(ms * 1000000);
#else
	Sleep(ms);
#endif
}