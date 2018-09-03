#include "install/install_nsp_remote.hpp"

#include <machine/endian.h>
#include "nx/fs.hpp"
#include "nx/ncm.hpp"
#include "util/title_util.hpp"
#include "debug.h"
#include "error.hpp"

namespace tin::install::nsp
{
    NetworkNSPInstallTask::NetworkNSPInstallTask(FsStorageId destStorageId, bool ignoreReqFirmVersion, std::string url) : IInstallTask(destStorageId, ignoreReqFirmVersion), m_remoteNSP(url)
    {
        m_remoteNSP.RetrieveHeader();
    }

    bool NetworkNSPInstallTask::ReadCNMT()
    {
        const PFS0FileEntry* fileEntry = m_remoteNSP.GetFileEntryByExtension("cnmt.nca");

        if (fileEntry == nullptr)
		{
            print("Failed to find cnmt file entry!\n");
			return false;
		}

        std::string cnmtNcaName(m_remoteNSP.GetFileEntryName(fileEntry));
        NcmNcaId cnmtNcaId = tin::util::GetNcaIdFromString(cnmtNcaName);
        size_t cnmtNcaSize = fileEntry->fileSize;

        nx::ncm::ContentStorage contentStorage(m_destStorageId);

        print("CNMT Name: %s\n", cnmtNcaName.c_str());

        // We install the cnmt nca early to read from it later
        this->InstallNCA(cnmtNcaId);
        std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtNcaId);

        // Create the cnmt filesystem
        nx::fs::IFileSystem cnmtNCAFileSystem;

        ASSERT_OK(cnmtNCAFileSystem.OpenFileSystemWithId(cnmtNCAFullPath, FsFileSystemType_ContentMeta, 0), ("Failed to open content meta file system " + cnmtNCAFullPath).c_str());

        tin::install::nsp::SimpleFileSystem cnmtNCASimpleFileSystem(cnmtNCAFileSystem, "/", cnmtNCAFullPath + "/");
        
        // Find and read the cnmt file
        auto cnmtName = cnmtNCASimpleFileSystem.GetFileNameFromExtension("", "cnmt");
        auto cnmtFile = cnmtNCASimpleFileSystem.OpenFile(cnmtName);
        u64 cnmtSize = cnmtFile.GetSize();

        m_cnmtByteBuf.resize(cnmtSize, 0);
        cnmtFile.Read(0x0, m_cnmtByteBuf.data(), cnmtSize);

        // Prepare cnmt content record
        m_cnmtContentRecord.ncaId = cnmtNcaId;
        *(u64*)m_cnmtContentRecord.size = cnmtNcaSize & 0xFFFFFFFFFFFF;
        m_cnmtContentRecord.type = NcmContentType_CNMT;

		return true;
    }

    bool NetworkNSPInstallTask::InstallNCA(const NcmNcaId& ncaId)
    {
        const PFS0FileEntry* fileEntry = m_remoteNSP.GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = m_remoteNSP.GetFileEntryName(fileEntry);
        size_t ncaSize = fileEntry->fileSize;

        print("Installing %s to storage Id %u\n", ncaFileName.c_str(), m_destStorageId);

        nx::ncm::ContentStorage contentStorage(m_destStorageId);

        // Attempt to delete any leftover placeholders
        contentStorage.DeletePlaceholder(ncaId);


        print("Size: 0x%lx\n", ncaSize);
        contentStorage.CreatePlaceholder(ncaId, ncaId, ncaSize);

        auto installBlockFunc = [&] (void* blockBuf, size_t bufSize, size_t blockStartOffset, size_t ncaSize)
        {
            contentStorage.WritePlaceholder(ncaId, blockStartOffset, blockBuf, bufSize);
            float progress = (float)blockStartOffset / (float)ncaSize;
            print("> Progress: %lu/%lu MB (%d%s)\r", (blockStartOffset / 1000000), (ncaSize / 1000000), (int)(progress * 100.0), "%");
        };

        //auto progressFunc = [&] (size_t sizeRead) {};

		if (!m_remoteNSP.RetrieveAndProcessNCA(ncaId, installBlockFunc, nullptr))
		{
			return false;
		}

        // Clean up the line for whatever comes next
        print("                                                           \r");
        print("Registering placeholder...\n");
        

		if(!contentStorage.Register(ncaId, ncaId))
		{
			print(("Failed to register " + ncaFileName + ". It may already exist.\n").c_str());
			return false;
		}

        contentStorage.DeletePlaceholder(ncaId);

		return true;
    }

    bool NetworkNSPInstallTask::InstallTicketCert()
    {        
        // Read the tik file and put it into a buffer
        const PFS0FileEntry* tikFileEntry = m_remoteNSP.GetFileEntryByExtension("tik");
        u64 tikSize = tikFileEntry->fileSize;
        auto tikBuf = std::make_unique<u8[]>(tikSize);
        print("> Reading tik\n");
        m_remoteNSP.m_download.BufferDataRange(tikBuf.get(), m_remoteNSP.GetDataOffset() + tikFileEntry->dataOffset, tikSize, nullptr);

        // Read the cert file and put it into a buffer
        const PFS0FileEntry* certFileEntry = m_remoteNSP.GetFileEntryByExtension("cert");
        u64 certSize = certFileEntry->fileSize;
        auto certBuf = std::make_unique<u8[]>(certSize);
        print("> Reading cert\n");
        m_remoteNSP.m_download.BufferDataRange(certBuf.get(), m_remoteNSP.GetDataOffset() + certFileEntry->dataOffset, certSize, nullptr);

        // Finally, let's actually import the ticket
        ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");

		return true;
    }

    bool NetworkNSPInstallTask::InstallCNMT()
    {
        // We manually install CNMTs early, so don't do it during
        // the install process
		return true;
    }
}