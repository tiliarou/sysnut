#include "nut.h"
#include "nx/ipc/ncm_ext.h"


static Service g_ncmSrv;
static u64 g_ncmRefCnt;

Result ncmextInitialize(void) 
{
#ifdef _MSC_VER
	return 0;
#else
    Result rc;
    atomicIncrement64(&g_ncmRefCnt);

    if (serviceIsActive(&g_ncmSrv))
        return 0;

    rc = smGetService(&g_ncmSrv, "ncm");

    if (R_SUCCEEDED(rc))
        smAddOverrideHandle(smEncodeName("ncm"), g_ncmSrv.handle);

    return rc;
#endif
}

void ncmextExit(void) {
#ifndef _MSC_VER
    if (atomicDecrement64(&g_ncmRefCnt) == 0) 
    {
        serviceClose(&g_ncmSrv);
    }
#endif
}

Result ncmCreatePlaceHolder(NcmContentStorage* cs, const NcaId* placeholderId, const NcaId* registeredId, u64 size)
{
#ifdef _MSC_VER
	return 0;
#else
    IpcCommand c;
    ipcInitialize(&c);
    
    struct Raw {
        u64 magic;
        u64 cmd_id;
        NcaId placeholder_id;
        NcaId registered_id;
        u64 size;
    } *raw;
    
    raw = (Raw*)ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    raw->size = size;
    memcpy(&raw->placeholder_id, placeholderId, sizeof(NcaId));
    memcpy(&raw->registered_id, registeredId, sizeof(NcaId));
    
    Result rc = serviceIpcDispatch(&cs->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        ipcParse(&r);

        struct Resp {
            u64 magic;
            u64 result;
        } *resp = (Resp*)r.Raw;

        rc = resp->result;
    }
    
    return rc;
#endif
}

Result ncmDeletePlaceHolder(NcmContentStorage* cs, const NcaId* placeholderId)
{
#ifdef _MSC_VER
	return 0;
#else
    IpcCommand c;
    ipcInitialize(&c);
    
    struct Raw {
        u64 magic;
        u64 cmd_id;
        NcaId placeholder_id;
    } *raw;
    
    raw = (Raw*)ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 2;
    memcpy(&raw->placeholder_id, placeholderId, sizeof(NcaId));
    
    Result rc = serviceIpcDispatch(&cs->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        ipcParse(&r);

        struct Resp {
            u64 magic;
            u64 result;
        } *resp = (Resp*)r.Raw;

        rc = resp->result;
    }
    
    return rc;
#endif
}

Result ncmWritePlaceHolder(NcmContentStorage* cs, const NcaId* placeholderId, u64 offset, void* buffer, size_t bufSize)
{
#ifdef _MSC_VER
	return 0;
#else
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendBuffer(&c, buffer, bufSize, BufferType_Normal);
    
    struct Raw {
        u64 magic;
        u64 cmd_id;
        NcaId placeholder_id;
        u64 offset;
    } *raw;
    
    raw = (Raw*)ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 4;
    raw->offset = offset;
    memcpy(&raw->placeholder_id, placeholderId, sizeof(NcaId));
    
    Result rc = serviceIpcDispatch(&cs->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        ipcParse(&r);

        struct Resp {
            u64 magic;
            u64 result;
        } *resp = (Resp*)r.Raw;

        rc = resp->result;
    }
    
    return rc;
#endif
}

Result ncmDelete(NcmContentStorage* cs, const NcaId* registeredId)
{
#ifdef _MSC_VER
	return 0;
#else
    IpcCommand c;
    ipcInitialize(&c);
    
    struct Raw {
        u64 magic;
        u64 cmd_id;
        NcaId placeholder_id;
    } *raw;
    
    raw = (Raw*)ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 6;
    memcpy(&raw->placeholder_id, registeredId, sizeof(NcaId));
    
    Result rc = serviceIpcDispatch(&cs->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        ipcParse(&r);

        struct Resp {
            u64 magic;
            u64 result;
        } *resp = (Resp*)r.Raw;

        rc = resp->result;
    }
    
    return rc;
#endif
}

Result ncmContentMetaDatabaseGetSize(NcmContentMetaDatabase* db, const NcmMetaRecord *record, u64* sizeOut) 
{
#ifdef _MSC_VER
	return 0;
#else
    IpcCommand c;
    ipcInitialize(&c);

    struct Raw {
        u64 magic;
        u64 cmd_id;
        NcmMetaRecord meta_record;
    } *raw;
    
    raw = (Raw*)ipcPrepareHeader(&c, sizeof(*raw));
    
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 10;
    memcpy(&raw->meta_record, record, sizeof(NcmMetaRecord));
    
    Result rc = serviceIpcDispatch(&db->s);
    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        ipcParse(&r);

        struct Resp {
            u64 magic;
            u64 result;
            u64 size_out;
        } *resp = (Resp*)r.Raw;

        rc = resp->result;

        if (R_SUCCEEDED(rc)) {
            if (sizeOut) *sizeOut = resp->size_out;
        }
    }
    
    return rc;
#endif
}