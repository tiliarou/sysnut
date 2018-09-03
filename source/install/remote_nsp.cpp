#include "install/remote_nsp.hpp"

#include "util/title_util.hpp"
#include "error.hpp"
#include "debug.h"

namespace tin::install::nsp
{
    RemoteNSP::RemoteNSP(std::string url) :
        m_download(url)
    {

    }

    // TODO: Do verification: PFS0 magic, sizes not zero
    bool RemoteNSP::RetrieveHeader()
    {
        print("Retrieving remote NSP header...\n");
		memset(m_headerBytes, 0, sizeof(m_headerBytes));
		m_headerSize = 0;

        // Retrieve the base header
		m_headerSize = sizeof(PFS0BaseHeader);
        m_download.BufferDataRange(m_headerBytes, 0x0, m_headerSize, nullptr);

        // Retrieve the full header
        size_t remainingHeaderSize = GetBaseHeader()->numFiles * sizeof(PFS0FileEntry) + GetBaseHeader()->stringTableSize;

		m_headerSize += remainingHeaderSize;

		if (m_headerSize >= MAX_NSP_HEADER_SIZE)
		{
			print("NSP header too large! %d bytes\n", m_headerSize);
			return false;
		}

        m_download.BufferDataRange(m_headerBytes + sizeof(PFS0BaseHeader), sizeof(PFS0BaseHeader), remainingHeaderSize, nullptr);
		print("Total header size: %d bytes\n", m_headerSize);
    }

    bool RemoteNSP::RetrieveAndProcessNCA(NcmNcaId ncaId, std::function<void (void* blockBuf, size_t bufSize, size_t blockStartOffset, size_t ncaSize)> processBlockFunc, std::function<void (size_t sizeRead)> progressFunc)
    {
        const PFS0FileEntry* fileEntry = GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = GetFileEntryName(fileEntry);

        print("Retrieving %s\n", ncaFileName.c_str());

        size_t ncaSize = fileEntry->fileSize;
        u64 fileOff = 0;
        size_t readSize = 0x20000; // 8MB buff
		u8* readBuffer = (u8*)malloc(readSize); // std::make_unique<u8[]>(readSize);

		if (readBuffer == NULL)
		{
			print("Failed to allocate read buffer for %s\n", ncaFileName.c_str());
			return false;
		}

        while (fileOff < ncaSize) 
        {   
            if (fileOff + readSize >= ncaSize) readSize = ncaSize - fileOff;
			print("read size: %d\n", readSize);

            m_download.BufferDataRange(readBuffer, this->GetDataOffset() + fileEntry->dataOffset + fileOff, readSize, progressFunc);

			print("finished chunk\n");

			if (processBlockFunc != nullptr)
			{
				processBlockFunc(readBuffer, readSize, fileOff, ncaSize);
			}

            fileOff += readSize;
        }

		free(readBuffer);

		print("Finished %s\n", ncaFileName.c_str());
		return true;
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntry(unsigned int index)
    {
		if (index >= this->GetBaseHeader()->numFiles)
			print("File entry index is out of bounds\n");
    
        size_t fileEntryOffset = sizeof(PFS0BaseHeader) + index * sizeof(PFS0FileEntry);

		if (m_headerSize < fileEntryOffset + sizeof(PFS0FileEntry))
		{
			print("Header bytes is too small to get file entry!");
		}

        return reinterpret_cast<PFS0FileEntry*>(m_headerBytes + fileEntryOffset);
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntryByExtension(std::string extension)
    {
        for (unsigned int i = 0; i < this->GetBaseHeader()->numFiles; i++)
        {
            const PFS0FileEntry* fileEntry = this->GetFileEntry(i);
            std::string name(this->GetFileEntryName(fileEntry));
            auto foundExtension = name.substr(name.find(".") + 1); 

			if (foundExtension == extension)
			{
				return fileEntry;
			}
        }

        return nullptr;
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntryByName(std::string name)
    {
        for (unsigned int i = 0; i < this->GetBaseHeader()->numFiles; i++)
        {
            const PFS0FileEntry* fileEntry = this->GetFileEntry(i);
            std::string foundName(this->GetFileEntryName(fileEntry));

			if (foundName == name)
			{
				return fileEntry;
			}
        }

        return nullptr;
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntryByNcaId(const NcmNcaId& ncaId)
    {
        const PFS0FileEntry* fileEntry = nullptr;
        std::string ncaIdStr = tin::util::GetNcaIdString(ncaId);

        if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".nca")) == nullptr)
        {
            if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".cnmt.nca")) == nullptr)
            {
                return nullptr;
            }
        }

        return fileEntry;
    }

    const char* RemoteNSP::GetFileEntryName(const PFS0FileEntry* fileEntry)
    {
        u64 stringTableStart = sizeof(PFS0BaseHeader) + this->GetBaseHeader()->numFiles * sizeof(PFS0FileEntry);
        return reinterpret_cast<const char*>(m_headerBytes + stringTableStart + fileEntry->stringTableOffset);
    }

    const PFS0BaseHeader* RemoteNSP::GetBaseHeader()
    {
		if (m_headerSize == 0)
		{
			print("Cannot retrieve header as header bytes are empty. Have you retrieved it yet?\n");
		}

        return reinterpret_cast<PFS0BaseHeader*>(m_headerBytes);
    }

    u64 RemoteNSP::GetDataOffset()
    {
        if (m_headerSize == 0)
		{
            print("Cannot get data offset as header is empty. Have you retrieved it yet?\n");
		}

        return m_headerSize;
    }
}