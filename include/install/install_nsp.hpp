#pragma once

#include <switch.h>
#include "install/content_meta.hpp"
#include "install/install.hpp"
#include "install/simple_filesystem.hpp"
#include "nx/ipc/tin_ipc.h"

namespace tin::install::nsp
{
    class NSPInstallTask : public IInstallTask
    {
        private:
            tin::install::nsp::SimpleFileSystem* const m_simpleFileSystem;

        protected:
            bool ReadCNMT() override;
			bool InstallNCA(const NcmNcaId& ncaId) override;
			bool InstallTicketCert() override;
			bool InstallCNMT() override;

        public:
            NSPInstallTask(tin::install::nsp::SimpleFileSystem& simpleFileSystem, FsStorageId destStorageId, bool ignoreReqFirmVersion);
    };
};

