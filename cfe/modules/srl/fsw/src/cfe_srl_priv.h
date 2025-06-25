#ifndef CFE_SRL_PRIV_H
#define CFE_SRL_PRIV_H

#include "cfe_srl_eventids.h"
#include "cfe_srl_api_typedefs.h"
// #include "cfe_srl_handle.h"
#include "cfe_srl_task.h"


CFE_SRL_IO_Handle_t *CFE_SRL_GetHandle(CFE_SRL_Handle_Indexer_t Index);
CFE_SRL_GPIO_Handle_t *CFE_SRL_GetGpioHandle(CFE_SRL_GPIO_Indexer_t Index);

int32 CFE_SRL_WriteGenericI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);
int32 CFE_SRL_WriteGenericUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);
int32 CFE_SRL_WriteGenericCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);
int32 CFE_SRL_WriteGenericSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);

int32 CFE_SRL_ReadGenericI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);
int32 CFE_SRL_ReadGenericCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);
int32 CFE_SRL_ReadGenericUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);
int32 CFE_SRL_ReadGenericSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params);

/*
 * External variables private to the serial module
 */
extern CFE_SRL_TaskData_t CFE_SRL_TaskData;

#endif /* CFE_SRL_PRIV_H */