#include "nx/fs.h"

Fs::Fs(nca_fs_header_t& header, nca_section_entry_t& sectionEntry) : File(), Directory()
{
	memcpy(dynamic_cast<nca_fs_header_t*>(this), &header, sizeof(header));
	memcpy(dynamic_cast<nca_section_entry_t*>(this), &sectionEntry, sizeof(sectionEntry));
}

Fs::~Fs()
{
}