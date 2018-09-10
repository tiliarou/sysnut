#pragma once

#include "nx/primitives.h"
#include "nx/integer.h"

Result ncmextInitialize(void);
void ncmextExit(void);

Result ncmCreatePlaceHolder(NcmContentStorage* cs, const NcaId* placeholderId, const NcaId* registeredId, u64 size);
Result ncmDeletePlaceHolder(NcmContentStorage* cs, const NcaId* placeholderId);
Result ncmWritePlaceHolder(NcmContentStorage* cs, const NcaId* placeholderId, u64 offset, void* buffer, size_t bufSize);
Result ncmDelete(NcmContentStorage* cs, const NcaId* registeredId);

Result ncmContentMetaDatabaseGetSize(NcmContentMetaDatabase* db, const NcmMetaRecord *record, u64* sizeOut);