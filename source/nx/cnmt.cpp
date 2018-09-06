#include "nx/cnmt.h"
#include "nx/crypto.h"
#include "nx/keys.h"
#include "log.h"
#include <string.h>

Cnmt::Cnmt() : Nca()
{
}

Cnmt::~Cnmt()
{
}

bool Cnmt::open(string& path, char* mode)
{
	if (!Nca::open(path, mode))
	{
		return false;
	}

	if (!size())
	{
		print("No file size %s\n", path.c_str());
		close();
		return false;
	}

	if (!read(buffer()))
	{
		print("Failed to read file %s\n", path.c_str());
		close();
		return false;
	}

	Crypto crypto(keys().headerKey, sizeof(keys().headerKey), MBEDTLS_CIPHER_AES_128_XTS);
	crypto.xtsDecrypt(buffer().buffer(), buffer().buffer(), 0x400, 0, 0x200);

	if (buffer().size() < sizeof(ContentMetaHeader))
	{
		print("Data size is too small! 0x%lx\n", buffer().size());
		return false;
	}
	print("success\n");

	/*
	for (unsigned int i = 0; i < contentMetaHeader()->contentCount; i++)
	{
		HashedContentRecord* record = this->hashedContentRecord(i);

		// Don't install delta fragments. Even patches don't seem to install them.
		if (record->record.contentType <= 5)
			m_contentRecords.push_back(hashedContentRecord.record);
	}
	*/

	return true;
}
