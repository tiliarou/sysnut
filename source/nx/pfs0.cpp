#include "nx/pfs0.h"
#include "log.h"

Pfs0::Pfs0() : Fs()
{
}

Pfs0::Pfs0(nca_fs_header_t& header, nca_section_entry_t& sectionEntry, Buffer<u8>& _key) : Fs(header, sectionEntry, _key)
{
}

bool Pfs0::init()
{
	print("pfs0\n");
	if (!Fs::init())
	{
		return false;
	}

	if (!read(m_header, sizeof(pfs0_header_t)))
	{
		error("Failed to read!\n");
		return false;
	}

	if (header().magic() != MAGIC_PFS0)
	{
		error("Invalid PFS0 Magic!  bad key?\n");
		close();
		return false;
	}

	m_header.dump();

	rewind();
	if (!read(m_header, header().size()))
	{
		error("Failed to read!\n");
		return false;
	}

	for (unsigned long i = 0; i < header().numFiles(); i++)
	{
		sptr<FileEntry> f(new Pfs0FileEntry(header().fileName(i), *header().fileEntry(i), this));
		files().push(f);
	}

	return true;
}

Pfs0::~Pfs0()
{
	print("~pfs0\n");
}

sptr<File> Pfs0::open(u64 i)
{
	pfs0_file_entry_t* entry = header().fileEntry((u32)i);
	
	sptr<File> f(new File());
	u64 sz = header().size();
	f->open2(ptr(), entry->offset + sz, entry->size);
	f->path() = header().fileName((u32)i);
	return f;
}