#include "install/install.hpp"

#include <cstring>
#include <memory>
#include "error.hpp"

namespace tin::install
{
    IInstallTask::IInstallTask(FsStorageId destStorageId, bool ignoreReqFirmVersion) : m_destStorageId(destStorageId), m_ignoreReqFirmVersion(ignoreReqFirmVersion)
    {
	}

    // Validate and obtain all data needed for install
    bool IInstallTask::PrepareForInstall()
    {
		if (!ReadCNMT() || !ParseCNMT())
		{
			return false;
		}

		return true;
    }

    bool IInstallTask::ParseCNMT()
    {
        // Parse data and create install content meta
        ASSERT_OK(m_contentMeta.ParseData(m_cnmtByteBuf.data(), m_cnmtByteBuf.size()), "Failed to parse data");

		if (m_ignoreReqFirmVersion)
		{
			print("WARNING: Required system firmware version is being IGNORED!\n");
		}

        ASSERT_OK(m_contentMeta.GetInstallContentMeta(&m_metaRecord, m_cnmtContentRecord, m_installContentMetaData, m_ignoreReqFirmVersion), "Failed to get install content meta");

		return true;
    }

    bool IInstallTask::Install()
    {
        Result rc = 0;
        std::vector<ContentStorageRecord> storageRecords;
        u64 baseTitleId = 0;
        u32 contentMetaCount = 0;

        // Updates and DLC don't share the same title id as the base game, but we
        // should be able to derive it manually.
        if (m_metaRecord.type == static_cast<u8>(ContentMetaType::APPLICATION))
        {
            baseTitleId = m_metaRecord.titleId;
        }
        else if (m_metaRecord.type == static_cast<u8>(ContentMetaType::PATCH))
        {
            baseTitleId = m_metaRecord.titleId ^ 0x800;
        }
        else if (m_metaRecord.type == static_cast<u8>(ContentMetaType::ADD_ON_CONTENT))
        {
            baseTitleId = (m_metaRecord.titleId ^ 0x1000) & ~0xFFF;
        }

        // TODO: Make custom error with result code field
        // 0x410: The record doesn't already exist
        if (R_FAILED(rc = nsCountApplicationContentMeta(baseTitleId, &contentMetaCount)) && rc != 0x410)
        {
            return false;
        }

        rc = 0;

        print("Content meta count: %u\n", contentMetaCount);

        // Obtain any existing app record content meta and append it to our vector
        if (contentMetaCount > 0)
        {
            storageRecords.resize(contentMetaCount);
            size_t contentStorageBufSize = contentMetaCount * sizeof(ContentStorageRecord);
            auto contentStorageBuf = std::make_unique<ContentStorageRecord[]>(contentMetaCount);
            u32 entriesRead;

            ASSERT_OK(nsListApplicationRecordContentMeta(0, baseTitleId, contentStorageBuf.get(), contentStorageBufSize, &entriesRead), "Failed to list application record content meta");

            if (entriesRead != contentMetaCount)
            {
				print("Mismatch between entries read and content meta count\n");
				return false;
            }

            memcpy(storageRecords.data(), contentStorageBuf.get(), contentStorageBufSize);
        }

        // Add our new content meta
        ContentStorageRecord storageRecord;
        storageRecord.metaRecord = m_metaRecord;
        storageRecord.storageId = m_destStorageId;
        storageRecords.push_back(storageRecord);

        print("Installing ticket and cert...\n");

        InstallTicketCert();



        // Replace the existing application records with our own - dont care if fails
        nsDeleteApplicationRecord(baseTitleId);


        print("Pushing application record...\n");
        ASSERT_OK(nsPushApplicationRecord(baseTitleId, 0x3, storageRecords.data(), storageRecords.size() * sizeof(ContentStorageRecord)), "Failed to push application record");

        print("Writing content records...\n");
        WriteRecords();

        // Install CNMT
        InstallCNMT();

        print("Installing NCAs...\n");
        for (auto& record : m_contentMeta.m_contentRecords)
        {
            InstallNCA(record.ncaId);
        }

		return true;
    }

    bool IInstallTask::WriteRecords()
    {
        NcmContentMetaDatabase contentMetaDatabase;

        ASSERT_OK(ncmOpenContentMetaDatabase(m_destStorageId, &contentMetaDatabase), "Failed to open content meta database");
        ASSERT_OK(ncmContentMetaDatabaseSet(&contentMetaDatabase, &m_metaRecord, m_installContentMetaData.size(), (NcmContentMetaRecordsHeader*)m_installContentMetaData.data()), "Failed to set content records");
        ASSERT_OK(ncmContentMetaDatabaseCommit(&contentMetaDatabase), "Failed to commit content records");

		return true;
    }
}