#include "nx/bufferedfile.h"

BufferedFile::BufferedFile() : File()
{
}

BufferedFile::~BufferedFile()
{
}

bool BufferedFile::setCrypto(crypt_type_t cryptoType, Buffer& key)
{
	crypto().type() = cryptoType;
	crypto().key() = key;
	return true;
}