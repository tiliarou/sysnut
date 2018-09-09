#include "nx/file.h"
#include "nx/nca.h"
#include "nx/pfs0.h"
#include "nx/cnmt.h"

File* fileFactoryCreate(string& path)
{
	if (path.endsWith(string(".nca")))
	{
		return new Nca();
	}
	else if (path.endsWith(string(".nsp")) || path.endsWith(string(".pfs0")))
	{
		return new Pfs0();
	}
	else if (path.endsWith(string(".cnmt")))
	{
		return new Cnmt();
	}

	return new File();
}

sptr<File> File::factory(string& path, char* mode)
{
	File* f = fileFactoryCreate(path);

	if (!f->open(path, mode))
	{
		return NULL;
	}

	return sptr<File>(f);
}

sptr<File> File::factory(string& path, sptr<File>& parent, u64 offset, u64 sz)
{
	File* f = fileFactoryCreate(path);

	f->path() = path;

	if (!f->open2(parent, offset, sz))
	{
		return NULL;
	}

	return sptr<File>(f);
}

File* File::factoryRawPtr(string& path, char* mode)
{
	File* f = fileFactoryCreate(path);

	if (!f->open(path, mode))
	{
		return NULL;
	}

	return f;
}

File* File::factoryRawPtr(string& path, sptr<File>& parent, u64 offset, u64 sz)
{
	File* f = fileFactoryCreate(path);

	f->path() = path;

	if (!f->open2(parent, offset, sz))
	{
		return NULL;
	}

	return f;
}