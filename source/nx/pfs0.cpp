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

	t.dump();

	return true;
}

Pfs0::~Pfs0()
{
	print("~pfs0\n");
}