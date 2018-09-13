#include "nx/ipc/ncm_ext.h"
#include "nx/contentstorage.h"
#include "log.h"


ContentStorage::ContentStorage(FsStorageId _storageId) 
{
	storageId() = _storageId;
#ifndef _MSC_VER
	if (ncmOpenContentStorage(_storageId, &m_contentStorage))
	{
		error("Failed to open NCM ContentStorage\n");
	}
#endif
}

ContentStorage::~ContentStorage()
{
#ifndef _MSC_VER
    serviceClose(&m_contentStorage.s);
#endif
}

bool ContentStorage::createPlaceholder(const NcaId& placeholderId, const NcaId& registeredId, size_t size)
{
#ifndef _MSC_VER
	if (ncmCreatePlaceHolder(&m_contentStorage, &placeholderId, &registeredId, size))
	{
		error("Failed to create placeholder\n");
		return false;
	}
#endif
	return true;
}
            
bool ContentStorage::deletePlaceholder(const NcaId &placeholderId)
{
#ifndef _MSC_VER
	if (ncmDeletePlaceHolder(&m_contentStorage, &placeholderId))
	{
		error("Failed to delete placeholder\n");
		return false;
	}
#endif
	return true;
}

bool ContentStorage::writePlaceholder(const NcaId &placeholderId, u64 offset, void *buffer, size_t bufSize)
{
#ifndef _MSC_VER
	if (ncmWritePlaceHolder(&m_contentStorage, &placeholderId, offset, buffer, bufSize))
	{
		error("Failed to write to placeholder\n");
		return false;
	}
#endif
	return true;
}

bool ContentStorage::reg(const NcaId &placeholderId, const NcaId &registeredId)
{
#ifndef _MSC_VER
	if (ncmContentStorageRegister(&m_contentStorage, &registeredId, &placeholderId))
	{
		error("Failed to register placeholder NCA\n");
		return false;
	}
#endif
	return true;
}

bool ContentStorage::del(const NcaId &registeredId)
{
#ifndef _MSC_VER
	if (ncmDelete(&m_contentStorage, &registeredId))
	{
		error("Failed to delete registered NCA\n");
		return false;
	}
#endif
	return true;
}

string ContentStorage::getPath(const NcaId& registeredId)
{
#ifndef _MSC_VER
	string result;
	if (ncmContentStorageGetPath(&m_contentStorage, &registeredId, pathBuf, FS_MAX_PATH))
	{
		error("Failed to get installed NCA path");
	}
    return result;
#else
	return hx(registeredId) + ".nca";
#endif
}
