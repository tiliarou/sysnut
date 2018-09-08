#include "nx/bufferedfile.h"
#include "log.h"

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

	switch (crypto().type())
	{
		case CRYPT_CTR:
			print("Crypt_ctr\n");
			crypto().setMode(MBEDTLS_CIPHER_AES_128_CTR);
			crypto().setKey(key.buffer(), key.size());
			break;
		case CRYPT_XTS:
			crypto().setMode(MBEDTLS_CIPHER_AES_128_XTS);
			crypto().setKey(key.buffer(), key.size());
			break;
		default:
			error("Unknown crypto type: %x\n", crypto().type());
	}
	return true;
}

bool BufferedFile::open(string& path, char* mode)
{
	return File::open(path, mode);
}

bool BufferedFile::close()
{
	return File::close();
}

bool BufferedFile::seek(u64 offset, int whence)
{
	currentPosition() = offset;
	if (crypto().type() == CRYPT_CTR)
	{
		crypto().updateCounter(currentPosition() + partitionOffset());
	}
	return true;
}

bool BufferedFile::seekThrough(u64 offset, int whence)
{
	if (crypto().type() == CRYPT_CTR)
	{
		crypto().updateCounter(offset + partitionOffset());
	}

	return File::seek(offset, whence);
}

bool BufferedFile::rewind()
{
	return seek(0);
}

u64 BufferedFile::read(Buffer& buffer, u64 sz)
{
	if (!sz || currentPosition() + sz > size())
	{
		sz = size() - currentPosition();
	}

	if (!page().contains(currentPosition(), sz))
	{
		page().load(this, currentPosition(), sz);
	}

	if (!page().slice(buffer, currentPosition() - page().pageOffset(), currentPosition() - page().pageOffset() + sz))
	{
		return false;
	}
	currentPosition() += sz;
	return sz;
}

u64 BufferedFile::readThrough(Buffer& buffer, u64 sz)
{
	if (crypto().type() == CRYPT_CTR)
	{
		crypto().updateCounter(currentPosition() + partitionOffset());
	}
	return File::read(buffer, sz);
}

u64 BufferedFile::tell()
{
	return currentPosition();
}

FileCrypto::FileCrypto() : Crypto()
{
	type() = CRYPT_NULL;
}

bool Page::contains(u64 offset, u64 sz)
{
	if (!active())
	{
		return false;
	}

	u64 pageId = offset / PAGE_ALIGNMENT;

	if (pageId != id())
	{
		return false;
	}

	if (offset + sz <= pageOffset() + size())
	{
		return true;
	}

	return false;
}

bool Page::load(BufferedFile* f, u64 offset, u64 sz)
{
	if (active() && dirty())
	{
		// flush page before unloading
	}

	id() = offset / PAGE_ALIGNMENT;
	u64 lastPage = (offset + sz) / PAGE_ALIGNMENT + 1;
	u64 pageSize = (lastPage - id()) * PAGE_ALIGNMENT;
	if (pageOffset() + pageSize > size())
	{
		pageSize = size() - pageOffset();
	}

	print("tell: %x, pageOffset = %x\n", (u32)f->tell(), (u32)pageOffset());
	f->seekThrough(pageOffset());
	u64 r = f->readThrough(*this, pageSize);

	if (!r)
	{
		return false;
	}

	switch(f->crypto().type())
	{
		case CRYPT_NULL:
		case CRYPT_NONE:
			break;
		case CRYPT_CTR:
			print("CTR selected\n");
			f->crypto().decrypt(this->buffer(), this->buffer(), this->size());
			break;
		default:
			error("Unknown crypto: %d\n", f->crypto().type());
			return false;
	}

	active() = true;

	return true;
}