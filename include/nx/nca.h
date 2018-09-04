#pragma once
#include "nut.h"
#include "nx/pfs0.h"
#include "nx/ivfc.h"
#include "nx/bktr.h"
#include "nx/file.h"

typedef struct {
	u8 c[0x10];
} NcmNcaId;

#define MAGIC_NCA3 0x3341434E /* "NCA3" */
#define MAGIC_NCA2 0x3241434E /* "NCA2" */
#define MAGIC_NCA0 0x3041434E /* "NCA0" */

typedef struct {
	uint32_t media_start_offset;
	uint32_t media_end_offset;
	uint8_t _0x8[0x8]; /* Padding. */
} nca_section_entry_t;

typedef struct {
	ivfc_hdr_t ivfc_header;
	uint8_t _0xE0[0x18];
	bktr_header_t relocation_header;
	bktr_header_t subsection_header;
} bktr_superblock_t;

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
	CRYPT_NCA0 = MAGIC_NCA0
} section_crypt_type_t;

/* NCA FS header. */
typedef struct {
	uint8_t _0x0;
	uint8_t _0x1;
	uint8_t partition_type;
	uint8_t fs_type;
	uint8_t crypt_type;
	uint8_t _0x5[0x3];
	union { /* FS-specific superblock. Size = 0x138. */
		pfs0_superblock_t pfs0_superblock;
		romfs_superblock_t romfs_superblock;
		//nca0_romfs_superblock_t nca0_romfs_superblock;
		bktr_superblock_t bktr_superblock;
	};
	union {
		uint8_t section_ctr[0x8];
		struct {
			uint32_t section_ctr_low;
			uint32_t section_ctr_high;
		};
	};
	uint8_t _0x148[0xB8]; /* Padding. */
} nca_fs_header_t;

/* Nintendo content archive header. */
typedef struct {
	uint8_t fixed_key_sig[0x100]; /* RSA-PSS signature over header with fixed key. */
	uint8_t npdm_key_sig[0x100]; /* RSA-PSS signature over header with key in NPDM. */
	uint32_t magic;
	uint8_t distribution; /* System vs gamecard. */
	uint8_t content_type;
	uint8_t crypto_type; /* Which keyblob (field 1) */
	uint8_t kaek_ind; /* Which kaek index? */
	uint64_t nca_size; /* Entire archive size. */
	uint64_t title_id;
	uint8_t _0x218[0x4]; /* Padding. */
	union {
		uint32_t sdk_version; /* What SDK was this built with? */
		struct {
			uint8_t sdk_revision;
			uint8_t sdk_micro;
			uint8_t sdk_minor;
			uint8_t sdk_major;
		};
	};
	uint8_t crypto_type2; /* Which keyblob (field 2) */
	uint8_t _0x221[0xF]; /* Padding. */
	uint8_t rights_id[0x10]; /* Rights ID (for titlekey crypto). */
	nca_section_entry_t section_entries[4]; /* Section entry metadata. */
	uint8_t section_hashes[4][0x20]; /* SHA-256 hashes for each section header. */
	uint8_t encrypted_keys[4][0x10]; /* Encrypted key area. */
	uint8_t _0x340[0xC0]; /* Padding. */
	nca_fs_header_t fs_headers[4]; /* FS section headers. */
} nca_header_t;

enum nca_section_type {
	PFS0,
	ROMFS,
	BKTR,
	NCA0_ROMFS,
	INVALID
};

enum nca_version {
	NCAVERSION_UNKNOWN = 0,
	NCAVERSION_NCA0_BETA,
	NCAVERSION_NCA0,
	/* NCAVERSION_NCA1, // Does this exist? */
	NCAVERSION_NCA2,
	NCAVERSION_NCA3
};

class NcaId
{
public:

private:
	u8 m_buffer[0x10];
};

class Nca : public File
{
public:
	Nca();

	bool open(string& path, char* mode = "rb");
	Buffer& header() { return m_header; }

private:
	Buffer m_header;
};