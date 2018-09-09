#include "nx/nca.h"
#include "nx/crypto.h"
#include "log.h"
#include "nx/keys.h"

Nca::Nca() : BufferedFile()
{
	path() = ".nca";
}

Nca::~Nca()
{
}

sptr<Fs> Nca::loadFs(nca_fs_header_t& fsHeader, nca_section_entry_t& sectionEntry, integer<128>& _key)
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

bool Nca::init()
{
	Buffer<u8> header;
	rewind();

	if (!read(header, 0xC00))
	{
		print("Failed to read file %s\n", path().c_str());
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
		// standard crypto
		auto& k = keys().keyAreaKey(masterKeyRev(), kaekIndex());
		Crypto crypto(k, sizeof(k), MBEDTLS_CIPHER_AES_128_ECB);
		crypto.decrypt(&m_keys, &m_keys, sizeof(m_keys));
		key() = m_keys[2];
	}
	else
	{
		// needs ticket
	}

	for (int i = 0; i < sizeof(fs_headers) / sizeof(nca_fs_header_t); i++)
	{
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