#ifndef CFE_SRL_HANDLE_H
#define CFE_SRL_HANDLE_H
#include <stdint.h>

#include "cfe_psp.h"
#include "iodriver_base.h"
#include "iodriver_discrete_io.h"
#include "iodriver_serial_io.h"
#include "cfe_srl_extern_typedefs.h"
#include "cfe_srl_interface_cfg.h"

typedef struct {
    CFE_SRL_IO_Handle_t Handle;
    char Name[CFE_SRL_HANDLE_NAME_LENGTH];      // Like "I2C2"
    char DevName[CFE_SRL_HANDLE_NAME_LENGTH];   // Like "/dev/i2c-2"
    CFE_SRL_DevType_t DevType;
    uint8_t MutexIdx;
    uint8_t Status;
} CFE_SRL_Global_Handle_t;

/**
 * Declaration of Struct & Values are located in `cfe_srl_basic.h`
*/
int CFE_SRL_PriorInit(void);

bool CFE_SRL_QueryStatus(const CFE_SRL_Global_Handle_t *Entry, CFE_SRL_Handle_Status_t Query);
int CFE_SRL_SetHandleStatus(CFE_SRL_IO_Handle_t *Handle, uint8_t Label, bool Set);
CFE_SRL_DevType_t CFE_SRL_GetHandleDevType(CFE_SRL_IO_Handle_t *Handle);

int CFE_SRL_ConfigHandle(uint8_t DevType, CFE_PSP_IODriver_Serial_cfg_t *Config);
int CFE_SRL_HandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *Devname, uint8_t DevType, uint8_t MutexIdx, CFE_PSP_IODriver_Serial_cfg_t *Config);
int CFE_SRL_HandleClose(CFE_SRL_IO_Handle_t **Handle);

int CFE_SRL_GpioInit(CFE_SRL_GPIO_Handle_t *Handle, const char *Path, unsigned int Line, const char *Name, bool Default, bool IsOut);
int CFE_SRL_GpioClose(CFE_SRL_GPIO_Handle_t *Handle);

#endif /* CFE_SRL_HANDLE_H */