#include "nx/nca.h"
#include "nx/crypto.h"
#include "log.h"
#include "nx/keys.h"

Nca::Nca() : BufferedFile()
{
	memset(&fs, 0, sizeof(fs));
}

Nca::~Nca()
{
}

Fs* Nca::loadFs(nca_fs_header_t& fsHeader, nca_section_entry_t& sectionEntry, Buffer& _key)
{
	switch (fsHeader.fs_type)
	{
		case FS_TYPE_PFS0:
		{
			Pfs0* fs = new Pfs0(fsHeader, sectionEntry, _key);
			fs->open2(this, fs->media_start_offset * MEDIA_SIZE + fs->superBlock().pfs0_offset, (fs->media_end_offset * MEDIA_SIZE) - (fs->media_start_offset * MEDIA_SIZE) - fs->superBlock().pfs0_offset);
			return fs;
		}
		default:
			return NULL;
	}
}

Fs** Nca::begin()
{
	return &fs[0];
}

Fs** Nca::end()
{
	for (int i = 0; i < 4; i++)
	{
		if (!fs[i])
		{
			return &fs[i];
		}
	}
	return &fs[4];
}

bool Nca::open(string& path, char* mode)
{
	Buffer header;

	if (!File::open(path))
	{
		return false;
	}

	if (!size())
	{
		print("No file size %s\n", path.c_str());
		close();
		return false;
	}


	if (!read(header, 0xC00))
	{
		print("Failed to read file %s\n", path.c_str());
		close();
		return false;
	}

	Crypto crypto(keys().headerKey, sizeof(keys().headerKey), MBEDTLS_CIPHER_AES_128_XTS);
	crypto.xtsDecrypt(header.buffer(), header.buffer(), 0xC00, 0, 0x200);
	memcpy(dynamic_cast<nca_header_t*>(this), header.buffer(), sizeof(nca_header_t));

	if (magic == MAGIC_NCA3)
	{
	}
	else
	{
		error("Unknown NCA magic (decryption failed?): %x\n", magic);
		close();
		return false;
	}

	for (int i = 0; i < sizeof(fs_headers) / sizeof(nca_fs_header_t); i++)
	{
		key().set("\xb4\x4e\x36\xd7\xf7\xc4\x44\x81\xf8\x5d\x2b\x5b\x64\x87\xa8\x1f", 0x10);
		fs[i] = loadFs(fs_headers[i], section_entries[i], key());
	}

	return true;
}

bool Nca::close()
{
	for (int i = 0; i < sizeof(fs_headers) / sizeof(nca_fs_header_t); i++)
	{
		if (fs[i])
		{
			delete fs[i];
			fs[i] = NULL;
		}
	}

	return File::close();
}