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

	return true;
}

Buffer<u8> Cnmt::contentMetaHeaderEx()
{
	return Buffer<u8>(reinterpret_cast<u8*>(buffer().buffer() + sizeof(ContentMetaHeader)), contentMetaHeader()->extendedHeaderSize);
}

NcmMetaRecord Cnmt::contentMetaKey() const
{
	NcmMetaRecord r;
	r.titleId = contentMetaHeader()->titleId;
	r.version = contentMetaHeader()->titleVersion;
	r.type = (u8)contentMetaHeader()->type;
	return r;
}

Buffer<u8> Cnmt::ncmContentMeta()
{
	Buffer<u8> buf;
	NcmContentMetaHeader header;

	header.extendedHeaderSize = contentMetaHeader()->extendedHeaderSize;
	header.contentCount = contentMetaHeader()->contentCount + 1;
	header.contentMetaCount = contentMetaHeader()->contentMetaCount;
	header.padding = 0;

	/*Buffer<u8> sep;
	sep.push(0xFE);
	sep.push(0xFE);
	sep.push(0xFE);
	sep.push(0xFE);*/

	buf += header;
	//buf += sep;
	buf += contentMetaHeaderEx();
	//buf += sep;

	ContentRecord cnmtContentRecord;
	cnmtContentRecord.contentType = ContentType::META;
	cnmtContentRecord.setSize(size());
	cnmtContentRecord.ncaId = 0;
	cnmtContentRecord.unk = 0;

	buf += cnmtContentRecord;
	//buf += sep;

	for (auto& record : *this)
	{
		buf += record.record;
		//buf += sep;
	}

	if (contentMetaHeader()->type == ContentMetaType::PATCH)
	{
		buf.resize(buf.size() + contentMetaHeaderEx<const PatchMetaExtendedHeader>()->extendedDataSize);
	}

	return buf;
}