#pragma once

#include "nx/file.h"
#include "nx/crypto.h"

class FileCrypto
{
public:
	crypt_type_t& type() { return m_type; }
	Buffer& key() { return m_key; }
private:
	crypt_type_t m_type;
	Buffer m_key;
};

class BufferedFile : public File
{
public:
	BufferedFile();
	virtual ~BufferedFile();

	FileCrypto& crypto() { return m_crypto; }
	bool setCrypto(crypt_type_t cryptoType, Buffer& key);
private:
	FileCrypto m_crypto;
};