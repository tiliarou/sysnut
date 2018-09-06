#include "nx/nca.h"
#include "nx/crypto.h"
#include "log.h"

Nca::Nca() : File()
{
}

bool Nca::open(string& path, char* mode)
{
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

	Buffer enc;

	if (!read(enc, 0xC00))
	{
		print("Failed to read file %s\n", path.c_str());
		close();
		return false;
	}

	header().resize(enc.size());

	Buffer key = uhx(HEADER_KEY, 32);
	Crypto crypto(key.buffer(), key.size(), MBEDTLS_CIPHER_AES_128_XTS);
	crypto.xtsDecrypt(header().buffer(), enc.buffer(), 0x400, 0, 0x200);

	return true;
}