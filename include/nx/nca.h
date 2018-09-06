#pragma once
#include "nut.h"
#include "nx/pfs0.h"
#include "nx/ivfc.h"
#include "nx/bktr.h"
#include "nx/file.h"
#include "nx/fs.h"

#define MEDIA_SIZE 0x200

typedef struct {
	u8 c[0x10];
} NcmNcaId;

#define MAGIC_NCA3 0x3341434E /* "NCA3" */
#define MAGIC_NCA2 0x3241434E /* "NCA2" */
#define MAGIC_NCA0 0x3041434E /* "NCA0" */

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

class Nca : public File, public nca_header_t
{
public:
	Nca();
	virtual ~Nca();

	virtual bool open(string& path, char* mode = "rb");
	virtual bool close();

private:
	Fs* loadFs(nca_fs_header_t& fsHeader, nca_section_entry_t& sectionEntry);
	Fs* fs[4];
};