#include "nx/cnmt.h"
#include "nx/crypto.h"
#include "nx/keys.h"
#include "log.h"
#include <string.h>

Cnmt::Cnmt() : File()
{
}

Cnmt::~Cnmt()
{
}

bool Cnmt::init()
{
	if (!File::init())
	{
		return false;
	}

	rewind();

	if (!read(buffer()))
	{
		print("Failed to read file %s\n", path().c_str());
		close();
		return false;
	}

	if (contentMetaHeader()->contentCount > 0x10)
	{
		error("Too many content records, perhaps bad cnmt file?\n");
		close();
		return false;
	}

	for (auto& content : *this)
	{
		print("content %x, %s\n", content.record.contentType, hx(content.record.ncaId).c_str());
	}

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
