#pragma once
#include "nut.h"
#include "nx/crypto.h"
#include "nx/bufferedfile.h"
#include "nx/directory.h"
#include "nx/integer.h"

typedef enum {
	PARTITION_ROMFS = 0,
	PARTITION_PFS0 = 1
} section_partition_type_t;

typedef enum {
	FS_TYPE_PFS0 = 2,
	FS_TYPE_ROMFS = 3
} section_fs_type_t;


class nca_fs_header_t
{
public:
	u8 _0x0;
	u8 _0x1;
	u8 partition_type;
	u8 fs_type;
	crypt_type_t crypt_type;
	u8 _0x5[0x3];
	u8 superblock_data[0x138];
	/*union { /* FS-specific superblock. Size = 0x138. *//*
		pfs0_superblock_t pfs0_superblock;
		romfs_superblock_t romfs_superblock;
		//nca0_romfs_superblock_t nca0_romfs_superblock;
		bktr_superblock_t bktr_superblock;
	};*/
	union {
		u8 section_ctr[0x8];
		struct {
			uint32_t section_ctr_low;
			uint32_t section_ctr_high;
		};
	};
	u8 _0x148[0xB8]; /* Padding. */
};

typedef struct {
	uint32_t media_start_offset;
	uint32_t media_end_offset;
	u8 _0x8[0x8]; /* Padding. */
} nca_section_entry_t;

class Fs : public nca_fs_header_t, public nca_section_entry_t, public BufferedFile, public Directory
{
public:
	Fs();
	Fs(nca_fs_header_t& header, nca_section_entry_t& sectionEntry, integer<128>& _key);
	virtual ~Fs();

	virtual bool init();

	section_fs_type_t& type() { return m_type; }
private:
	section_fs_type_t m_type;
};