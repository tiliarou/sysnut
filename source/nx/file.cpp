#include "nut.h"
#include "log.h"
#include "nx/file.h"
#include "nx/diskfile.h"
#include "nx/curlfile.h"
#include "mbedtls/sha256.h"

void nullDeleter(File*) {}

File::File() : m_ptr(this, &nullDeleter)
{
	memset(m_children, 0, sizeof(m_children));
}

File::~File()
{
	close();
}

bool File::copy(string src, string dst)
{
	FILE* f1 = fopen(src, "rb");
	if (!f1)
	{
		error("Failed to open file %s\n", src.c_str());
		return false;
	}

	FILE* f2 = fopen(dst, "wb");

	if (!f2)
	{
		fclose(f1);
		error("Failed to open file %s\n", dst.c_str());
		return false;
	}

	size_t bytesRead = 0;
	char buffer[0x1000];

	do
	{
		bytesRead = fread(&buffer, 1, sizeof(buffer), f1);

		if (bytesRead)
		{
			fwrite(buffer, 1, bytesRead, f2);
		}
	}
	while (bytesRead > 0);
	return true;
}

bool File::open(Url path, const char* mode)
{
	if (isOpen())
	{
		warning("opening file with closing handle first %s\n", path);
		close();
	}

	this->path() = path;

	File* fp = NULL;

	if (this->path().scheme() == "ftp")
	{
		fp = new CurlFile();
	}
	else
	{
		fp = new DiskFile();
	}

	sptr<File> f(fp);


	if (!f->open(path, mode))
	{
		error("failed to open file %s\n", path.c_str());
		return false;
	}

	setParent(reinterpret_cast<sptr<File>&>(f));

	if (!init())
	{
		return false;
	}

	return true;
}

bool File::open2(sptr<File>& f, u64 offset, u64 sz)
{
	if (!sz)
	{
		sz = f->size() - offset;
	}

	setParent(f);
	partitionOffset() = offset;
	partitionSize() = sz;

	//print("created partition offset = %x, size = %x\n", offset, sz);
	return init();
}

u64 File::read(Buffer<u8>& buffer, u64 sz)
{
	if (!isOpen())
	{
		error("tried to read from closed file\n");
		return 0;
	}

	if (!sz)
	{
		sz = size() - tell();
	}

	return m_parent->read(buffer, sz);
}

bool File::isOpen()
{
	return (bool)m_parent;
}

u64 File::size()
{
	if (!isOpen())
	{
		error("tried to size on closed file\n");
		return false;
	}

	if (isPartition())
	{
		return partitionSize();
	}

	return m_parent->size();
}

u64 File::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	u64 pos = parent()->tell();

	if (partitionOffset() > pos)
	{
		return 0;
	}

	return pos - partitionOffset();
}

bool File::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}

	switch (whence)
	{
		case SEEK_SET:
			return parent()->seek(offset + partitionOffset(), SEEK_SET);
		case SEEK_CUR:
			return parent()->seek(offset, SEEK_CUR);
		case SEEK_END:
			return parent()->seek(size() - partitionOffset() - offset, SEEK_SET);
	}

	return false;
}

bool File::rewind()
{
	return seek(0);
}

bool File::close()
{
	if (!isOpen())
	{
		return false;
	}

	m_parent->unregisterChild(this);

	m_parent.reset();

	return true;
}

bool File::init()
{
	if (!isOpen())
	{
		return false;
	}

	return true;
}

bool File::setParent(sptr<File> parent)
{
	if (isOpen())
	{
		close();
	}

	// todo notify parent
	m_parent = parent;
	m_parent->registerChild(this);
	return true;
}

u64 File::childrenCount()
{
	u64 c = 0;

	for (int i = 0; i < MAX_FILE_CHILDREN; i++)
	{
		if (m_children[i])
		{
			c++;
		}
	}
	return c;
}

bool File::registerChild(File* child)
{
	for (int i = 0; i < MAX_FILE_CHILDREN; i++)
	{
		if (!m_children[i])
		{
			m_children[i] = child;
			return true;
		}
	}
	return false;
}

bool File::unregisterChild(File* child)
{
	for (int i = 0; i < MAX_FILE_CHILDREN; i++)
	{
		if (m_children[i] == child)
		{
			m_children[i] = NULL;

			if (childrenCount() == 0)
			{
				onChildless();
			}
			return true;
		}
	}
	return false;
}

void File::onChildless()
{
}

integer<256> File::sha256()
{
	integer<256> result(0);

	if (!isOpen())
	{
		return result;
	}

	mbedtls_sha256_context ctx;
	mbedtls_sha256_init(&ctx);

	Buffer<u8> buffer;

	rewind();

	const u64 chunkSize = 0x100000;

	while (read(buffer, chunkSize))
	{
		mbedtls_sha256_update(&ctx, buffer.buffer(), buffer.size());
	}

	mbedtls_sha256_finish(&ctx, (u8*)&result);

	mbedtls_sha256_free(&ctx);
	return result;
}