/************************************************************************
 * Author : Hyeokjin Kweon
 * 
 * Last Modified : 2025 - 04 - 27
 * 
 * Purpose : Serial Comm. Core Module's Base header
 ************************************************************************/

#ifndef CFE_SRL_BASE_H
#define CFE_SRL_BASE_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "cfe.h"
#include <gpiod.h>
#include "cfe_srl_eventids.h"

#include "cfe_srl_api_typedefs.h"
#include "cfe_srl_mission_cfg.h"

typedef enum {
    SRL_DEVTYPE_I2C = 1,
    SRL_DEVTYPE_SPI,
    SRL_DEVTYPE_CAN,
    SRL_DEVTYPE_UART,
    SRL_DEVTYPE_RS422
} CFE_SRL_DevType_t;


typedef struct {
    CFE_SRL_IO_Handle_t Handle;
    char Name[CFE_SRL_HANDLE_NAME_LENGTH];      // Like "UANT_I2C"
    char DevName[CFE_SRL_HANDLE_NAME_LENGTH];   // Like "/dev/i2c-2"
    CFE_SRL_DevType_t DevType;
    uint8_t MutexID;
    uint8_t Status;
} CFE_SRL_Global_Handle_t;

typedef enum {
    CFE_SRL_HANDLE_STATUS_NONE = 0x00,
    CFE_SRL_HANDLE_STATUS_ALLOCATE = 0x01,
    CFE_SRL_HANDLE_STATUS_FD_INIT = 0x02,
    CFE_SRL_HANDLE_STATUS_MUTEX_INIT = 0x04,
    CFE_SRL_HANDLE_STATUS_ALL = 0x07
} CFE_SRL_Handle_Status_t;


int CFE_SRL_BasicOpen(const char *Device, int Option);

ssize_t CFE_SRL_BasicWrite(int FD, const void *Data, size_t Size);

ssize_t CFE_SRL_BasicRead(int FD, void *Data, size_t Size);

int CFE_SRL_BasicClose(int FD);

int CFE_SRL_BasicIOCTL(int FD, unsigned long Request, void *Data);

ssize_t CFE_SRL_BasicPollRead(int FD, void *Data, size_t Size, uint32_t Timeout);

CFE_SRL_DevType_t CFE_SRL_GetHandleDevType(CFE_SRL_IO_Handle_t *Handle);

bool CFE_SRL_QueryStatus(const CFE_SRL_Global_Handle_t *Entry, CFE_SRL_Handle_Status_t Query);

int CFE_SRL_SetHandleStatus(CFE_SRL_IO_Handle_t *Handle, uint8_t Label, bool Set);


/*************************************************************
 *  Basic UART/RS422 Function
 *************************************************************/
int CFE_SRL_BasicSetUART(CFE_SRL_IO_Handle_t *Handle, uint32_t BaudRate);
int CFE_SRL_ChangeBaudUART(CFE_SRL_IO_Handle_t *Handle, uint32_t BaudRate);


/*************************************************************
 *  Basic GPIO Function
 *************************************************************/
int32 CFE_SRL_BasicGpioOpen(CFE_SRL_GPIO_Handle_t *Handle, const char *Path);
int32 CFE_SRL_BasicGpioGetLine(CFE_SRL_GPIO_Handle_t *Handle, unsigned int Line);
int32 CFE_SRL_BasicGpioSetOutput(CFE_SRL_GPIO_Handle_t *Handle, const char *Name, bool Default);
int32 CFE_SRL_BasicGpioSetValue(CFE_SRL_GPIO_Handle_t *Handle, bool Value);
int32 CFE_SRL_BasicGpioClose(CFE_SRL_GPIO_Handle_t *Handle);

#endif /* CFE_SRL_BASE_H */