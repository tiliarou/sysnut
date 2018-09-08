#pragma once

#include "nut.h"
#include "nx/string.h"
#include "nx/file.h"
#include "nx/nca.h"

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
	NcaId ncaId;
	u8 size[0x6];
	u8 contentType;
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
	Buffer<u8>& buffer() { return m_buffer; }

	ContentMetaHeader* contentMetaHeader() { return reinterpret_cast<ContentMetaHeader*>(buffer().buffer());  }
	HashedContentRecord* hashedContentRecord(u64 i) { return &(reinterpret_cast<HashedContentRecord*>(buffer().c_str() + sizeof(ContentMetaHeader) + contentMetaHeader()->extendedHeaderSize)[i]);  }

	HashedContentRecord* begin() { return hashedContentRecord(0); }
	HashedContentRecord* end() { return hashedContentRecord(contentMetaHeader()->contentCount); }
private:
	Buffer<u8> m_buffer;
};