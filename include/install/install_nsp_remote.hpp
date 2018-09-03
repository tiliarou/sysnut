#pragma once

#include <switch.h>
#include <string>
#include "install/install.hpp"
#include "install/remote_nsp.hpp"

namespace tin::install::nsp
{
    class NetworkNSPInstallTask : public IInstallTask
    {
        private:
            RemoteNSP m_remoteNSP;

        protected:
			bool ReadCNMT() override;
			bool InstallNCA(const NcmNcaId& ncaId) override;
			bool InstallTicketCert() override;
			bool InstallCNMT() override;

        public:
            NetworkNSPInstallTask(FsStorageId destStorageId, bool ignoreReqFirmVersion, std::string url);
    };
}