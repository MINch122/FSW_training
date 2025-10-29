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

#include "cfe_rf_api_typedefs.h"

int32 CFE_RF_CommandIngestInit(CFE_ES_TaskId_t *TaskIdPtr);
void CFE_RF_CommandIngestTask(void);


/// @brief Telemetry output function. Must only used in To app
/// @param BufPtr Tx Buffer
/// @param Size TxSize
/// @param Port Destination Port
/// @return `1` on success, `0` on failure.
/// @note This function Transmit via #STRX. Can be differed by rtable
int32 CFE_RF_TelemetryEmit(void *BufPtr, size_t Size, uint8_t Port);


/// @brief Telemetry output function. Must only used in To app
/// @param BufPtr Tx Buffer
/// @param Size TxSize
/// @param Port Destination Port
/// @return `1` on success, `0` on failure.
/// @note This function Transmit via #UTRX. Can be differed by rtable
int32 CFE_RF_TelemetryEmit2(void *BufPtr, size_t Size, uint8_t Port);

#endif