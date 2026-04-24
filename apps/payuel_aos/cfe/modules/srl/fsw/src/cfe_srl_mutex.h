#ifndef CFE_SRL_MUTEX_H
#define CFE_SRL_MUTEX_H

#include <pthread.h>
#include <stdbool.h>

#include "cfe_srl_module_all.h"

typedef struct {
    // Index : `MutexIdx`
    osal_id_t MutexId;
    bool Isinit;
} CFE_SRL_IO_Handle_Mutex_t;


int CFE_SRL_GlobalHandleMutexInit(void);
int CFE_SRL_GlobalHandleMutexLock(void);
int CFE_SRL_GlobalHandleMutexUnlock(void);

int CFE_SRL_SetHandleMutexID(CFE_SRL_IO_Handle_t *Handle, uint8_t MutexIdx);
int CFE_SRL_HandleMutexInit(CFE_SRL_IO_Handle_t *Handle, uint8_t MutexIdx, const char *Name);

int CFE_SRL_MutexLock(CFE_SRL_IO_Handle_t *Handle);
int CFE_SRL_MutexUnlock(CFE_SRL_IO_Handle_t *Handle);
int CFE_SRL_MutexDestroy(CFE_SRL_IO_Handle_t *Handle);

#endif /* CFE_SRL_MUTEX_H */