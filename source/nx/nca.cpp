#include "nx/nca.h"
#include "nx/crypto.h"
#include "log.h"
#include "nx/keys.h"

Nca::Nca() : BufferedFile()
{
}

Nca::~Nca()
{
}

sptr<Fs> Nca::loadFs(nca_fs_header_t& fsHeader, nca_section_entry_t& sectionEntry, Buffer<u8>& _key)
{
	switch (fsHeader.fs_type)
	{
		case FS_TYPE_PFS0:
		{
			Pfs0* p = NULL;
			sptr<Fs> fs((p = new Pfs0(fsHeader, sectionEntry, _key)));
			fs->open2(ptr(), p->media_start_offset * MEDIA_SIZE + p->superBlock().pfs0_offset, (p->media_end_offset * MEDIA_SIZE) - (p->media_start_offset * MEDIA_SIZE) - p->superBlock().pfs0_offset);
			return fs;
		}
		default:
			return NULL;
	}
}

bool Nca::open(string& path, char* mode)
{
	Buffer<u8> header;

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

	if (rightsId() == integer<128>(0))
	{
		print("standard crypto\n");
		auto& k = keys().keyAreaKey(masterKeyRev(), kaekIndex());
		Crypto crypto(k, sizeof(k), MBEDTLS_CIPHER_AES_128_ECB);
		crypto.decrypt(&m_keys, &m_keys, sizeof(m_keys));
	}
	else
	{
		// needs ticket
	}

	for (int i = 0; i < sizeof(fs_headers) / sizeof(nca_fs_header_t); i++)
	{
		key().set("\xb4\x4e\x36\xd7\xf7\xc4\x44\x81\xf8\x5d\x2b\x5b\x64\x87\xa8\x1f", 0x10);
		auto d = loadFs(fs_headers[i], section_entries[i], key());

		if (d)
		{
			directories().push(d);
		}
	}

	return true;
}

bool Nca::close()
{
	return File::close();
}