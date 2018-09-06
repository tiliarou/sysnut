#include "nx/pfs0.h"
#include "log.h"

Pfs0::Pfs0(nca_fs_header_t& header, nca_section_entry_t& sectionEntry) : Fs(header, sectionEntry)
{
	print("pfs0\n");
	print("%d files\n", this->numFiles());
}

Pfs0::~Pfs0()
{
	print("~pfs0\n");
}