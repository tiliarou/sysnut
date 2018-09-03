#include "nx/cnmt.h"
#include "log.h"
#include <string.h>

Cnmt::Cnmt() : File()
{
}

Cnmt::~Cnmt()
{
}

bool Cnmt::open(string path)
{
	u8 buffer[MAX_CNMT_SIZE];
	u64 bufferSize;

	if (!File::open(path))
	{
		return false;
	}

	bufferSize = size();

	if (!bufferSize)
	{
		close();
		return false;
	}

	if (bufferSize >= MAX_CNMT_SIZE)
	{
		close();
		return false;
	}

	memset(buffer, 0, sizeof(buffer));

	if(!read())

	if (bufferSize < sizeof(ContentMetaHeader))
	{
		print("Data size is too small! 0x%lx", bufferSize);
		return false;
	}

	memcpy(&m_contentMetaHeader, m_contentMetaBytes.data(), sizeof(ContentMetaHeader));

	for (unsigned int i = 0; i < m_contentMetaHeader.contentCount; i++)
	{
		HashedContentRecord hashedContentRecord = reinterpret_cast<HashedContentRecord*>(m_contentMetaBytes.data() + sizeof(ContentMetaHeader) + m_contentMetaHeader.extendedHeaderSize)[i];

		// Don't install delta fragments. Even patches don't seem to install them.
		if (hashedContentRecord.record.contentType <= 5)
			m_contentRecords.push_back(hashedContentRecord.record);
	}

	return true;
}

bool Cnmt::close()
{
	return true;
}