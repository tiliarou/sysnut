#pragma once
#include "nut.h"
#include "nx/file.h"
#include "nx/directory.h"

typedef enum {
	PARTITION_ROMFS = 0,
	PARTITION_PFS0 = 1
} section_partition_type_t;

typedef enum {
	FS_TYPE_PFS0 = 2,
	FS_TYPE_ROMFS = 3
} section_fs_type_t;

typedef enum {
	CRYPT_NONE = 1,
	CRYPT_XTS = 2,
	CRYPT_CTR = 3,
	CRYPT_BKTR = 4,
	CRYPT_NCA0 = 0x3041434E //MAGIC_NCA0
} section_crypt_type_t;

/* NCA FS header. */
typedef struct {
	uint8_t _0x0;
	uint8_t _0x1;
	uint8_t partition_type;
	uint8_t fs_type;
	uint8_t crypt_type;
	uint8_t _0x5[0x3];
	uint8_t superblock_data[0x138];
	/*union { /* FS-specific superblock. Size = 0x138. *//*
		pfs0_superblock_t pfs0_superblock;
		romfs_superblock_t romfs_superblock;
		//nca0_romfs_superblock_t nca0_romfs_superblock;
		bktr_superblock_t bktr_superblock;
	};*/
	union {
		uint8_t section_ctr[0x8];
		struct {
			uint32_t section_ctr_low;
			uint32_t section_ctr_high;
		};
	};
	uint8_t _0x148[0xB8]; /* Padding. */
} nca_fs_header_t;

typedef struct {
	uint32_t media_start_offset;
	uint32_t media_end_offset;
	uint8_t _0x8[0x8]; /* Padding. */
} nca_section_entry_t;

class Fs : public nca_fs_header_t, public nca_section_entry_t, public File, public Directory
{
public:
	Fs(nca_fs_header_t& header, nca_section_entry_t& sectionEntry);
	virtual ~Fs();

	section_fs_type_t& type() { return m_type; }
private:
	section_fs_type_t m_type;
};