#include "nx/cnmt.h"
#include "nx/crypto.h"
#include "log.h"
#include <string.h>

Cnmt::Cnmt() : File()
{
}

Cnmt::~Cnmt()
{
}

bool Cnmt::open(string& path, char* mode)
{
	if (!File::open(path))
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
	print("key size: %d\n", sizeof(HEADER_KEY));
	Crypto crypto(HEADER_KEY, 32, MBEDTLS_CIPHER_AES_128_XTS);
	//Buffer tmp(buffer());
	crypto.xtsDecrypt(buffer().buffer(), buffer().buffer(), buffer().size(), 0, 0x200);

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
