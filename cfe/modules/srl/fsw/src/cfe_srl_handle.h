#ifndef CFE_SRL_HANDLE_H
#define CFE_SRL_HANDLE_H


#define CFE_SRL_STUB
#undef CFE_SRL_STUB

#include <stdint.h>

#ifdef CFE_SRL_STUB
#include "cfe_srl_io_stub.h"
#else
#include "cfe_srl_io.h"
#endif

#include "cfe_srl_mutex.h"

/**
 * Declaration of Struct & Values are located in `cfe_srl_basic.h`
*/
int CFE_SRL_PriorInit(void);

int CFE_SRL_GetOpenOption(CFE_SRL_DevType_t Devtype);
int CFE_SRL_GlobalHandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *DevName, uint8_t DevType);
int CFE_SRL_HandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *Devname, uint8_t DevType, uint8_t MutexID, uint32_t BaudRate, uint8_t SPIMode);
int CFE_SRL_HandleClose(CFE_SRL_IO_Handle_t *Handle);

#endif /* CFE_SRL_HANDLE_H */