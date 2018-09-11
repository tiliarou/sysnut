#pragma once

#include "nut.h"
#include "nx/string.h"
#include "nx/file.h"
#include "nx/nca.h"
#include "nx/primitives.h"

#define MAX_CNMT_SIZE 0x4000

enum class ContentMetaType : u8
{
	SYSTEM_PROGRAM = 0x1,
	SYSTEM_DATA = 0x2,
	SYSTEM_UPDATE = 0x3,
	BOOT_IMAGE_PACKAGE = 0x4,
	BOOT_IMAGE_PACKAGE_SAFE = 0x5,
	APPLICATION = 0x80,
	PATCH = 0x81,
	ADD_ON_CONTENT = 0x82,
	DELTA = 0x83
};

enum class ContentType : u8
{
	META = 0x0,
	PROGRAM = 0x1,
	DATA = 0x2,
	CONTROL = 0x3,
	HTML_DOCUMENT = 0x4,
	LEGAL_INFORMATION = 0x5,
	DELTA_FRAGMENT = 0x6
};

struct NcmContentMetaHeader
{
	u16 extendedHeaderSize;
	u16 contentCount;
	u16 contentMetaCount;
	u16 padding;
} PACKED;

struct ContentMetaHeader
{
	u64 titleId;
	u32 titleVersion;
	ContentMetaType type;
	u8 padding0;
	u16 extendedHeaderSize;
	u16 contentCount;
	u16 contentMetaCount;
	u8 attributes;
	u8 padding1[0x3];
	u32 requiredDownloadSystemVersion;
	u8 padding2[0x4];
} PACKED;

static_assert(sizeof(ContentMetaHeader) == 0x20, "ContentMetaHeader must be 0x20!");

struct ApplicationMetaExtendedHeader
{
	u64 patchTitleId;
	u32 requiredSystemVersion;
	u32 padding;
} PACKED;

struct PatchMetaExtendedHeader
{
	u64 applicationTitleId;
	u32 requiredSystemVersion;
	u32 extendedDataSize;
	u8 padding[0x8];
} PACKED;

struct AddOnContentMetaExtendedHeader
{
	u64 applicationTitleId;
	u32 requiredApplicationVersion;
	u32 padding;
} PACKED;

struct ContentRecord
{
public:
	u64 size()
	{
		u64 result = 0;
		memcpy(&result, &m_size, sizeof(m_size));
		return result;
	}

	u64 setSize(u64 val)
	{
		memcpy(&m_size, &val, sizeof(m_size));
		return val;
	}

	NcaId ncaId;
	u8 m_size[0x6];
	ContentType contentType;
	u8 unk;
} PACKED;

static_assert(sizeof(ContentRecord) == 0x18, "ContentRecord must be 0x18!");

struct HashedContentRecord
{
	u8 hash[0x20];
	ContentRecord record;
} PACKED;

struct InstallContentMetaHeader
{
	u16 extendedHeaderSize;
	u16 contentCount;
	u16 contentMetaCount;
	u16 padding;
} PACKED;

class Cnmt : public File
{
public:
	Cnmt();
	~Cnmt();

	bool init() override;

	const Buffer<u8>& buffer() const { return m_buffer; }
	Buffer<u8>& buffer() { return m_buffer; }

	const ContentMetaHeader* contentMetaHeader() const { return reinterpret_cast<const ContentMetaHeader*>(buffer().buffer()); }
	ContentMetaHeader* contentMetaHeader() { return reinterpret_cast<ContentMetaHeader*>(buffer().buffer());  }
	Buffer<u8> contentMetaHeaderEx();
	HashedContentRecord* hashedContentRecord(u64 i) { return &(reinterpret_cast<HashedContentRecord*>(buffer().c_str() + sizeof(ContentMetaHeader) + contentMetaHeader()->extendedHeaderSize)[i]);  }

	HashedContentRecord* begin() { return hashedContentRecord(0); }
	HashedContentRecord* end() { return hashedContentRecord(contentMetaHeader()->contentCount); }

	Buffer<u8> ncmContentMeta();

	NcmMetaRecord contentMetaKey() const;

	template<class T = u8>
	T* contentMetaHeaderEx()
	{
		return reinterpret_cast<T*>(buffer().buffer(sizeof(ContentMetaHeader)));
	}
private:
	Buffer<u8> m_buffer;
};