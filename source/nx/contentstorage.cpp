#include "nx/contentstorage.h"


ContentStorage::ContentStorage(FsStorageId storageId) 
{
    //ASSERT_VOID(ncmOpenContentStorage(storageId, &m_contentStorage), "Failed to open NCM ContentStorage");
}

ContentStorage::~ContentStorage()
{
    //serviceClose(&m_contentStorage.s);
}

bool ContentStorage::createPlaceholder(const NcaId &placeholderId, const NcaId &registeredId, size_t size)
{
    //ASSERT_OK(ncmCreatePlaceHolder(&m_contentStorage, &placeholderId, &registeredId, size), "Failed to create placeholder");
	return true;
}
            
bool ContentStorage::deletePlaceholder(const NcaId &placeholderId)
{
	//ASSERT_OK(ncmDeletePlaceHolder(&m_contentStorage, &placeholderId), "Failed to delete placeholder");
	return true;
}

bool ContentStorage::writePlaceholder(const NcaId &placeholderId, u64 offset, void *buffer, size_t bufSize)
{
	//ASSERT_OK(ncmWritePlaceHolder(&m_contentStorage, &placeholderId, offset, buffer, bufSize), "Failed to write to placeholder");
	return true;
}

bool ContentStorage::reg(const NcaId &placeholderId, const NcaId &registeredId)
{
	//ASSERT_OK(ncmContentStorageRegister(&m_contentStorage, &registeredId, &placeholderId), "Failed to register placeholder NCA");
	return true;
}

bool ContentStorage::del(const NcaId &registeredId)
{
	//ASSERT_OK(ncmDelete(&m_contentStorage, &registeredId), "Failed to delete registered NCA");
	return true;
}

string ContentStorage::getPath(const NcaId &registeredId)
{
	string result;
    //ASSERT_STR(ncmContentStorageGetPath(&m_contentStorage, &registeredId, pathBuf, FS_MAX_PATH), "Failed to get installed NCA path");
    return result;
}
