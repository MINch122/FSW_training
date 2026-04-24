#include "cfe_srl_module_all.h"

int CFE_SRL_GlobalHandleMutexInit(void) {
    int Status;

    Status = OS_MutSemCreate(&CFE_SRL_Global.GlobalMutexId, "CFE_SRL_GlobalMutex", 0);
    if (Status != 0) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    return CFE_SUCCESS;
}

int CFE_SRL_GlobalHandleMutexLock(void) {
    int Status;
    
    // Status = pthread_mutex_lock(&GlobalHandleMutex);
    Status = OS_MutSemTake(CFE_SRL_Global.GlobalMutexId);
    
    return Status < 0 ? CFE_STATUS_EXTERNAL_RESOURCE_FAIL : CFE_SUCCESS;
}

int CFE_SRL_GlobalHandleMutexUnlock(void) {
    int Status;

    // Status = pthread_mutex_unlock(&GlobalHandleMutex);
    Status = OS_MutSemGive(CFE_SRL_Global.GlobalMutexId);

    return Status < 0 ? CFE_STATUS_EXTERNAL_RESOURCE_FAIL : CFE_SUCCESS;
}

int CFE_SRL_SetHandleMutexID(CFE_SRL_IO_Handle_t *Handle, uint8_t MutexIdx) {
    if (Handle == NULL) return -1; // Revise to `NULL_ERROR`

    CFE_SRL_Global_Handle_t *Entry;
    Entry = (CFE_SRL_Global_Handle_t *)Handle;

    Entry->MutexIdx = MutexIdx;
    
    return CFE_SUCCESS;
}

int CFE_SRL_HandleMutexInit(CFE_SRL_IO_Handle_t *Handle, uint8_t MutexIdx, const char *Name) {
    int Status;
    char MutexName[32] = {0};

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT; // Revise to `NULL_ERROR`

    if (MutexIdx > CFE_SRL_GNRL_DEVICE_NUM) {
        OS_printf("cFE SRL Handle Mutex Init Error. Mutex ID %u is NOT effective.\n", MutexIdx);
        return -1; // Revise to `MUTEX_NUM_ERR`
    }

    if (CFE_SRL_Global.IOMutex[MutexIdx].Isinit) {
        CFE_SRL_SetHandleMutexID(Handle, MutexIdx);
        Status = CFE_SRL_SetHandleStatus(Handle, CFE_SRL_HANDLE_STATUS_MUTEX_INIT, true);
        return CFE_SUCCESS;
    }
    snprintf(MutexName, sizeof(MutexName), "%s_IOMutex", Name);
    Status = OS_MutSemCreate(&CFE_SRL_Global.IOMutex[MutexIdx].MutexId, MutexName, 0);
    if (Status != 0) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    CFE_SRL_Global.IOMutex[MutexIdx].Isinit = true;
    CFE_SRL_SetHandleMutexID(Handle, MutexIdx);

    Status = CFE_SRL_SetHandleStatus(Handle, CFE_SRL_HANDLE_STATUS_MUTEX_INIT, true);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int CFE_SRL_MutexLock(CFE_SRL_IO_Handle_t *Handle) {
    int Status;
    
    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    const CFE_SRL_Global_Handle_t *Entry = (CFE_SRL_Global_Handle_t *)Handle;

    if (CFE_SRL_Global.IOMutex[Entry->MutexIdx].Isinit == false) return -1; // Revise to `MUTEX_NOT_INIT_ERR`

    Status = OS_MutSemTake(CFE_SRL_Global.IOMutex[Entry->MutexIdx].MutexId);
    if (Status < 0) {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

int CFE_SRL_MutexUnlock(CFE_SRL_IO_Handle_t *Handle) {
    int Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    const CFE_SRL_Global_Handle_t *Entry = (CFE_SRL_Global_Handle_t *)Handle;

    if (CFE_SRL_Global.IOMutex[Entry->MutexIdx].Isinit == false) return -1; // Revise to `MUTEX_NOT_INIT_ERR`

    Status = OS_MutSemGive(CFE_SRL_Global.IOMutex[Entry->MutexIdx].MutexId);
    if (Status < 0) {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

int CFE_SRL_MutexDestroy(CFE_SRL_IO_Handle_t *Handle) {
    int Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    const CFE_SRL_Global_Handle_t *Entry = (CFE_SRL_Global_Handle_t *)Handle;

    if (CFE_SRL_Global.IOMutex[Entry->MutexIdx].Isinit == false) return CFE_SUCCESS;
    else {
        Status = OS_MutSemDelete(CFE_SRL_Global.IOMutex[Entry->MutexIdx].MutexId);
        if (Status < 0) {
            return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }
        CFE_SRL_Global.IOMutex[Entry->MutexIdx].Isinit = false;
        return CFE_SUCCESS;
    }
}