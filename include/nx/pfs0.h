#pragma once
#include "nut.h"
#include "nx/fs.h"

#define MAGIC_PFS0 0x30534650

typedef struct {
	uint64_t offset;
	uint64_t size;
	uint32_t string_table_offset;
	uint32_t reserved;
} pfs0_file_entry_t;

typedef struct {
public:
	u64 size() { return sizeof(pfs0_header_t) + stringTableSize() + numFiles() * sizeof(pfs0_file_entry_t); }

	const char* stringTable()
	{
		return reinterpret_cast<const char*>((char*)this + sizeof(pfs0_header_t) + stringTableSize() + numFiles() * sizeof(pfs0_file_entry_t));
	}

	string& fileName(u32 i)
	{
		pfs0_file_entry_t* entry = fileEntry(i);

		u32 sz;

		if (numFiles() && i < numFiles() - 1)
		{
			pfs0_file_entry_t* nextEntry = fileEntry(i+1);
			sz = nextEntry->string_table_offset - entry->string_table_offset;
		}
		else
		{
			sz = stringTableSize() - entry->string_table_offset;
		}

		string s;
		s.set(stringTable() + entry->string_table_offset, sz);
		return s;
	}

	pfs0_file_entry_t* fileEntry(u32 i = 0)
	{
		return reinterpret_cast<pfs0_file_entry_t*>((char*)this + sizeof(pfs0_header_t) + (i * sizeof(pfs0_file_entry_t)));
	}

	u32& magic() { return m_magic; }
	u32& numFiles() { return m_numFiles; }
	u32& stringTableSize() { return m_stringTableSize; }
private:
    u32 m_magic;
    u32 m_numFiles;
    u32 m_stringTableSize;
    u32 reserved;
} pfs0_header_t;

typedef struct {
    uint8_t master_hash[0x20]; /* SHA-256 hash of the hash table. */
    uint32_t block_size; /* In bytes. */
    uint32_t always_2;
    uint64_t hash_table_offset; /* Normally zero. */
    uint64_t hash_table_size; 
    uint64_t pfs0_offset;
    uint64_t pfs0_size;
    uint8_t _0x48[0xF0];
} pfs0_superblock_t;

class Pfs0 : public Fs, public pfs0_header_t
{
public:
	Pfs0(nca_fs_header_t& header, nca_section_entry_t& sectionEntry, Buffer& _key);
	virtual ~Pfs0();

	virtual bool init();

	pfs0_superblock_t& superBlock() { return *reinterpret_cast<pfs0_superblock_t*>(&superblock_data);  }
private:
};
