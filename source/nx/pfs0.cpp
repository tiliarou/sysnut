#include "nx/pfs0.h"
#include "log.h"

Pfs0::Pfs0() : Fs()
{
	path() = ".pfs0";
}

Pfs0::Pfs0(nca_fs_header_t& header, nca_section_entry_t& sectionEntry, integer<128>& _key) : Fs(header, sectionEntry, _key)
{
	path() = ".pfs0";
}

bool Pfs0::init()
{
	if (!Fs::init())
	{
		return false;
	}
	rewind();

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
}
