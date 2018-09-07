#pragma once
#include "nut.h"
#include "nx/primitives.h"
#include "nx/pfs0.h"
#include "nx/ivfc.h"
#include "nx/bktr.h"
#include "nx/bufferedfile.h"
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
	u8 fixed_key_sig[0x100]; /* RSA-PSS signature over header with fixed key. */
	u8 npdm_key_sig[0x100]; /* RSA-PSS signature over header with key in NPDM. */
	uint32_t magic;
	u8 distribution; /* System vs gamecard. */
	u8 content_type;
	u8 crypto_type; /* Which keyblob (field 1) */
	u8 kaek_ind; /* Which kaek index? */
	u64 nca_size; /* Entire archive size. */
	u64 title_id;
	u8 _0x218[0x4]; /* Padding. */
	union {
		uint32_t sdk_version; /* What SDK was this built with? */
		struct {
			u8 sdk_revision;
			u8 sdk_micro;
			u8 sdk_minor;
			u8 sdk_major;
		};
	};
	u8 crypto_type2; /* Which keyblob (field 2) */
	u8 _0x221[0xF]; /* Padding. */
	u8 rights_id[0x10]; /* Rights ID (for titlekey crypto). */
	nca_section_entry_t section_entries[4]; /* Section entry metadata. */
	u8 section_hashes[4][0x20]; /* SHA-256 hashes for each section header. */
	u8 encrypted_keys[4][0x10]; /* Encrypted key area. */
	u8 _0x340[0xC0]; /* Padding. */
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

class Nca : public BufferedFile, public nca_header_t
{
public:
	Nca();
	virtual ~Nca();

	bool open(string& path, char* mode = "rb") override;
	virtual bool close() override;

	Buffer& key() { return m_key; }

	Fs* begin();
	Fs* end();

private:
	Fs* loadFs(nca_fs_header_t& fsHeader, nca_section_entry_t& sectionEntry, Buffer& _key);
	Fs* fs[4];
	Buffer m_key;
};