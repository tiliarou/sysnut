#pragma once

#include <switch.h>
#include <vector>

#include "install/content_meta.hpp"
#include "install/install.hpp"
#include "install/simple_filesystem.hpp"
#include "nx/ipc/tin_ipc.h"

namespace tin::install
{
    class IInstallTask
    {
        protected:
            const FsStorageId m_destStorageId;
            bool m_ignoreReqFirmVersion = false;

            std::vector<u8> m_cnmtByteBuf;
            NcmContentRecord m_cnmtContentRecord;
            tin::install::ContentMeta m_contentMeta;
            
            NcmMetaRecord m_metaRecord;
            std::vector<u8> m_installContentMetaData;

            IInstallTask(FsStorageId destStorageId, bool ignoreReqFirmVersion);

            virtual bool ReadCNMT() = 0;
            virtual bool ParseCNMT();

            virtual bool WriteRecords();
            virtual bool InstallTicketCert() = 0;
            virtual bool InstallNCA(const NcmNcaId &ncaId) = 0;
            virtual bool InstallCNMT() = 0;

        public:
            virtual bool PrepareForInstall();
            virtual bool Install();
    };
}