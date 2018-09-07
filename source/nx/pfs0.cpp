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

	print("header size: %x\n", this->size());

	for (int i = 0; i < header().numFiles(); i++)
	{
		string n = header().fileName(i);
		print("file: %s\n", n.c_str());
	}

	return true;
}

Pfs0::~Pfs0()
{
	print("~pfs0\n");
}