#pragma once
#include "nut.h"
#include "nx/primitives.h"
#include "nx/integer.h"
#include "nx/pfs0.h"
#include "nx/ivfc.h"
#include "nx/bktr.h"
#include "nx/bufferedfile.h"
#include "nx/directory.h"
#include "nx/fs.h"

#define MEDIA_SIZE 0x200

typedef struct {
	u8 c[0x10];
} NcmNcaId;

#define MAGIC_NCA3 0x3341434E /* "NCA3" */
#define MAGIC_NCA2 0x3241434E /* "NCA2" */
#define MAGIC_NCA0 0x3041434E /* "NCA0" */

/* Nintendo content archive header. */
class nca_header_t
{
public:
	u64& titleId() { return m_titleId; }
	integer<128>& rightsId() { return m_rightsId; }
	u8& cryptoType() { return m_cryptoType; }
	u8& cryptoType2() { return m_cryptoType2; }
	u8& kaekIndex() { return m_kaekIndex; }

	u8 masterKeyRev()
	{
		u8 r = MAX(cryptoType(), cryptoType2());
		if (r)
		{
			r--;
		}
		return r;
	}

	integer<2048> fixed_key_sig; /* RSA-PSS signature over header with fixed key. */
	integer<2048> npdm_key_sig; /* RSA-PSS signature over header with key in NPDM. */
	u32 magic;
	u8 distribution; /* System vs gamecard. */
	u8 content_type;
	u8 m_cryptoType; /* Which keyblob (field 1) */
	u8 m_kaekIndex; /* Which kaek index? */
	u64 nca_size; /* Entire archive size. */
	u64 m_titleId;
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
	u8 m_cryptoType2; /* Which keyblob (field 2) */
	u8 _0x221[0xF]; /* Padding. */
	integer<128> m_rightsId; /* Rights ID (for titlekey crypto). */
	nca_section_entry_t section_entries[4]; /* Section entry metadata. */
	integer<256> section_hashes[4]; /* SHA-256 hashes for each section header. */
	integer<128> m_keys[4]; /* Encrypted key area. */
	u8 _0x340[0xC0]; /* Padding. */
	nca_fs_header_t fs_headers[4]; /* FS section headers. */
};

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

class Nca : public BufferedFile, public nca_header_t, public Directory
{
public:
	Nca();
	virtual ~Nca();

	bool open(string& path, char* mode = "rb") override;
	virtual bool close() override;

	Buffer<u8>& key() { return m_key; }

private:
	sptr<Fs> loadFs(nca_fs_header_t& fsHeader, nca_section_entry_t& sectionEntry, Buffer<u8>& _key);
	//sptr<Fs> fs[4];
	Buffer<u8> m_key;
};
