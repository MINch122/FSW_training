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

inline int32 CFE_RF_TelemetryEmit(void *BufPtr, size_t Size, uint8_t Port) {
    return CFE_SRL_ApiTransactionCSP(CSP_NODE_GS_KISS, Port, BufPtr, Size, NULL, 0);
}

#endif