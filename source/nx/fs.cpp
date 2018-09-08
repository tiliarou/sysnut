#include "nx/fs.h"
#include "log.h"

Fs::Fs(nca_fs_header_t& header, nca_section_entry_t& sectionEntry, Buffer& _key) : BufferedFile(), Directory()
{
	memcpy(dynamic_cast<nca_fs_header_t*>(this), &header, sizeof(header));
	memcpy(dynamic_cast<nca_section_entry_t*>(this), &sectionEntry, sizeof(sectionEntry));

	crypto().key() = _key;
}

bool Fs::init()
{
	if (!BufferedFile::init())
	{
		return false;
	}

	setCrypto(this->crypt_type, crypto().key());
	//print("setting iv: %d\n", sizeof(section_ctr));
	crypto().setCounter(section_ctr, sizeof(section_ctr));

	return true;
}

Fs::~Fs()
{
}