#pragma once

#include "nut.h"
#include "nx/string.h"
#include "nx/integer.h"

class ContentStorage final
{
    private:
        NcmContentStorage m_contentStorage;

    public:
        // Don't allow copying, or garbage may be closed by the destructor
        ContentStorage& operator=(const ContentStorage&) = delete;
        ContentStorage(const ContentStorage&) = delete;   

        ContentStorage(FsStorageId storageId = FsStorageId_SdCard);
        ~ContentStorage();

        bool createPlaceholder(const NcaId &placeholderId, const NcaId &registeredId, size_t size);
        bool deletePlaceholder(const NcaId &placeholderId);
        bool writePlaceholder(const NcaId &placeholderId, u64 offset, void *buffer, size_t bufSize);
        bool reg(const NcaId& placeholderId, const NcaId &registeredId);
        bool del(const NcaId& registeredId);
        string getPath(const NcaId &registeredId);
};