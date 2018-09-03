#pragma once

#include <string>
#include <switch.h>

#include "nx/ipc/tin_ipc.h"

namespace nx::ncm
{
    class ContentStorage final
    {
        private:
            NcmContentStorage m_contentStorage;

        public:
            // Don't allow copying, or garbage may be closed by the destructor
            ContentStorage& operator=(const ContentStorage&) = delete;
            ContentStorage(const ContentStorage&) = delete;   

            ContentStorage(FsStorageId storageId);
            ~ContentStorage();

            bool CreatePlaceholder(const NcmNcaId &placeholderId, const NcmNcaId &registeredId, size_t size);
            bool DeletePlaceholder(const NcmNcaId &placeholderId);
            bool WritePlaceholder(const NcmNcaId &placeholderId, u64 offset, void *buffer, size_t bufSize);
            bool Register(const NcmNcaId &placeholderId, const NcmNcaId &registeredId);
            bool Delete(const NcmNcaId &registeredId);
            std::string GetPath(const NcmNcaId &registeredId);
    };
}