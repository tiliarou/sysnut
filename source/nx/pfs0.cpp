#include "nx/pfs0.h"
#include "log.h"

Pfs0::Pfs0(nca_fs_header_t& header, nca_section_entry_t& sectionEntry, Buffer& _key) : Fs(header, sectionEntry, _key)
{
}

bool Pfs0::init()
{
	print("pfs0\n");
	if (!Fs::init())
	{
		return false;
	}

	//print("%d files\n", this->numFiles());

	Buffer t;
	if (!read(t))
	{
		error("Failed to read!\n");
	}

	memcpy(dynamic_cast<pfs0_header_t*>(this), t.buffer(), sizeof(pfs0_header_t));

	if (magic() != MAGIC_PFS0)
	{
		error("Invalid PFS0 Magic!  bad key?\n");
		close();
		return false;
	}

	t.dump();


	return true;
}

Pfs0::~Pfs0()
{
	print("~pfs0\n");
}