#include "nx/file.h"
#include "nx/nca.h"
#include "nx/pfs0.h"
#include "nx/cnmt.h"
#include "nx/primitives.h"

template < > const char* mapTypeExtension<Nca>() { return ".nca"; }
template < > const char* mapTypeExtension<Pfs0>() { return ".nsp"; }
template < > const char* mapTypeExtension<Cnmt>() { return ".cnmt"; }

File* fileFactoryCreate(Url& path)
{
	if (path.str().endsWith(string(".nca")))
	{
		return new Nca();
	}
	else if (path.str().endsWith(string(".nsp")) || path.str().endsWith(string(".pfs0")))
	{
		return new Pfs0();
	}
	else if (path.str().endsWith(string(".cnmt")))
	{
		return new Cnmt();
	}

	return new File();
}

sptr<File> File::factory(Url& path, const char* mode)
{
	File* f = fileFactoryCreate(path);

	if (!f->open(path, mode))
	{
		return NULL;
	}

	return sptr<File>(f);
}

sptr<File> File::factory(Url& path, sptr<File>& parent, u64 offset, u64 sz)
{
	File* f = fileFactoryCreate(path);

	f->path() = path;

	if (!f->open2(parent, offset, sz))
	{
		return NULL;
	}

	return sptr<File>(f);
}

File* File::factoryRawPtr(Url& path, const char* mode)
{
	File* f = fileFactoryCreate(path);

	if (!f->open(path, mode))
	{
		return NULL;
	}

	return f;
}

File* File::factoryRawPtr(Url& path, sptr<File>& parent, u64 offset, u64 sz)
{
	File* f = fileFactoryCreate(path);

	f->path() = path;

	if (!f->open2(parent, offset, sz))
	{
		return NULL;
	}

	return f;
}