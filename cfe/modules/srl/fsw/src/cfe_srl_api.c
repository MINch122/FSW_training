/******************************************************************************
** File: cfe_srl_api.c
**
** Purpose:
**      This file contains the source code for the SRL API's.
**
** Notes: The following 4 terms have been, or are used in the cFS architecture and implementation
**
**      Handle -
**
**      GlobalHandle  -
**
**      MutexID - 
**
**
** Author:   HyeokJin Kweon
**
** P.S.: Declarations of function are located at
**       `cfe/modules/core_api/fsw/inc/cfe_srl.h`
******************************************************************************/

/*
** Include Files
*/
#include "cfe_srl_module_all.h"

/*----------------------------------------------------------------
 *
 * Serial Write API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
CFE_SRL_IO_Handle_t *CFE_SRL_ApiGetHandle(CFE_SRL_Handle_Indexer_t Index) {
    return CFE_SRL_GetHandle(Index);
}

CFE_SRL_GPIO_Handle_t *CFE_SRL_ApiGetGpioHandle(CFE_SRL_GPIO_Indexer_t Index) {
    return CFE_SRL_GetGpioHandle(Index);
}


/*----------------------------------------------------------------
 *
 * Serial Write API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ApiWrite(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    if (Handle == NULL || Handle->Func.TxFunc == NULL ||Params->TxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    return Handle->Func.TxFunc(Handle, Params);
}


/*----------------------------------------------------------------
 *
 * Serial Read API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ApiRead(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    if (Handle == NULL || Handle->Func.RxFunc == NULL || 
        Params->TxData == NULL || Params->RxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    return Handle->Func.RxFunc(Handle, Params);
}


/*----------------------------------------------------------------
 *
 * Device Close API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ApiClose(CFE_SRL_IO_Handle_t *Handle) {
    int32 Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = CFE_SRL_HandleClose(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}


/*----------------------------------------------------------------
 *
 * GPIO setting API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ApiGpioSet(CFE_SRL_GPIO_Handle_t *Handle, bool Value) {
    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;
    
    return CFE_SRL_BasicGpioSetValue(Handle, Value);
}

/*----------------------------------------------------------------
 *
 * CSP Transaction API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ApiTransactionCSP(uint8_t Node, uint8_t Port, void *TxData, int TxSize, void *RxData, int RxSize) {
    int32 Status;

    if (TxData == NULL || RxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = CFE_SRL_TransactionCSP(Node, Port, TxData, TxSize, RxData, RxSize);
    if (Status == CFE_SRL_TRANSACTION_ERR) return Status;

    return Status; // 1 or reply size
}

/*----------------------------------------------------------------
 *
 * CSP Rparam API
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ApiGetRparamCSP(uint8_t Type, uint8_t Node, uint8_t TableId, uint16_t Addr, void *Param) {
    return CFE_SRL_GetRparamCSP(Type, Node, TableId, Addr, Param);
}
int32 CFE_SRL_ApiSetRparamCSP(uint8_t Type, uint8_t Node, uint8_t TableId, uint16_t Addr, void *Param) {
    return CFE_SRL_SetRparamCSP(Type, Node, TableId, Addr, Param);
}