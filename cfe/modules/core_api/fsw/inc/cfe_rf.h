/******************************************************************************
** File: cfe_rf.h
**
** Purpose:
**      This file contains the definitions of cFE Radio communication
**      Application Programmer's Interface
**
**
** Author:   HyeokJin Kweon
**
** P.S.: Source code of functions is located at
**       `cfe/modules/rf/fsw/src/cfe_rf_api.c`
******************************************************************************/

#ifndef CFE_RF_H
#define CFE_RF_H


int32 CFE_RF_CommandIngestInit(CFE_ES_TaskId_t *TaskIdPtr);

int32 CFE_RF_TelemetryEmit(void *BufPtr, size_t Size, uint8_t Port);

#endif