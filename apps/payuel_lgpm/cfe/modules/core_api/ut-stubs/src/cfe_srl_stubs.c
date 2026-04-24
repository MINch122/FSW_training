/***********************************************************************
 *  Copyright (c) 2025, Yonsei University as represented by the
 *  Department of Satellite Systems (DSS) & Astrodynamics & Control Lab (ACL)
 *  All rights reserved. This software was created at DSS
 *************************************************************************/

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in cfe_srl header
 */

#include "cfe_srl.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiChangeVia()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiChangeVia(uint8_t Via)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiChangeVia, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiChangeVia, uint8_t, Via);

    UT_GenStub_Execute(CFE_SRL_ApiChangeVia, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiChangeVia, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiClose()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiClose(CFE_SRL_IO_Handle_t *Handle)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiClose, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiClose, CFE_SRL_IO_Handle_t *, Handle);

    UT_GenStub_Execute(CFE_SRL_ApiClose, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiClose, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiGetGpioHandle()
 * ----------------------------------------------------
 */
CFE_SRL_GPIO_Handle_t *CFE_SRL_ApiGetGpioHandle(CFE_SRL_GPIO_Indexer_t Index)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiGetGpioHandle, CFE_SRL_GPIO_Handle_t *);

    UT_GenStub_AddParam(CFE_SRL_ApiGetGpioHandle, CFE_SRL_GPIO_Indexer_t, Index);

    UT_GenStub_Execute(CFE_SRL_ApiGetGpioHandle, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiGetGpioHandle, CFE_SRL_GPIO_Handle_t *);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiGetHandle()
 * ----------------------------------------------------
 */
CFE_SRL_IO_Handle_t *CFE_SRL_ApiGetHandle(CFE_SRL_Handle_Indexer_t Index)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiGetHandle, CFE_SRL_IO_Handle_t *);

    UT_GenStub_AddParam(CFE_SRL_ApiGetHandle, CFE_SRL_Handle_Indexer_t, Index);

    UT_GenStub_Execute(CFE_SRL_ApiGetHandle, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiGetHandle, CFE_SRL_IO_Handle_t *);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiGetRparamCSP()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiGetRparamCSP(uint8_t Type, uint8_t Node, uint8_t TableId, uint16_t Addr, void *Param)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiGetRparamCSP, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiGetRparamCSP, uint8_t, Type);
    UT_GenStub_AddParam(CFE_SRL_ApiGetRparamCSP, uint8_t, Node);
    UT_GenStub_AddParam(CFE_SRL_ApiGetRparamCSP, uint8_t, TableId);
    UT_GenStub_AddParam(CFE_SRL_ApiGetRparamCSP, uint16_t, Addr);
    UT_GenStub_AddParam(CFE_SRL_ApiGetRparamCSP, void *, Param);

    UT_GenStub_Execute(CFE_SRL_ApiGetRparamCSP, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiGetRparamCSP, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiGpioGet()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiGpioGet(CFE_SRL_GPIO_Handle_t *Handle, bool *Value)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiGpioGet, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiGpioGet, CFE_SRL_GPIO_Handle_t *, Handle);
    UT_GenStub_AddParam(CFE_SRL_ApiGpioGet, bool *, Value);

    UT_GenStub_Execute(CFE_SRL_ApiGpioGet, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiGpioGet, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiGpioSet()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiGpioSet(CFE_SRL_GPIO_Handle_t *Handle, bool Value)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiGpioSet, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiGpioSet, CFE_SRL_GPIO_Handle_t *, Handle);
    UT_GenStub_AddParam(CFE_SRL_ApiGpioSet, bool, Value);

    UT_GenStub_Execute(CFE_SRL_ApiGpioSet, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiGpioSet, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiPingCSP()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiPingCSP(uint8 Node, uint32 Timeout, unsigned int Size, uint8 Options)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiPingCSP, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiPingCSP, uint8, Node);
    UT_GenStub_AddParam(CFE_SRL_ApiPingCSP, uint32, Timeout);
    UT_GenStub_AddParam(CFE_SRL_ApiPingCSP, unsigned int, Size);
    UT_GenStub_AddParam(CFE_SRL_ApiPingCSP, uint8, Options);

    UT_GenStub_Execute(CFE_SRL_ApiPingCSP, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiPingCSP, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiPrintRtable()
 * ----------------------------------------------------
 */
void CFE_SRL_ApiPrintRtable(void)
{

    UT_GenStub_Execute(CFE_SRL_ApiPrintRtable, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiRead()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiRead(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiRead, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiRead, CFE_SRL_IO_Handle_t *, Handle);
    UT_GenStub_AddParam(CFE_SRL_ApiRead, CFE_SRL_IO_Param_t *, Params);

    UT_GenStub_Execute(CFE_SRL_ApiRead, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiRead, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiRparamSaveCSP()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiRparamSaveCSP(uint8 Node, uint32 Timeout, uint8 TableId, uint8 To)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiRparamSaveCSP, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiRparamSaveCSP, uint8, Node);
    UT_GenStub_AddParam(CFE_SRL_ApiRparamSaveCSP, uint32, Timeout);
    UT_GenStub_AddParam(CFE_SRL_ApiRparamSaveCSP, uint8, TableId);
    UT_GenStub_AddParam(CFE_SRL_ApiRparamSaveCSP, uint8, To);

    UT_GenStub_Execute(CFE_SRL_ApiRparamSaveCSP, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiRparamSaveCSP, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiSetRparamCSP()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiSetRparamCSP(uint8_t Type, uint8_t Node, uint8_t TableId, uint16_t Addr, void *Param)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiSetRparamCSP, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiSetRparamCSP, uint8_t, Type);
    UT_GenStub_AddParam(CFE_SRL_ApiSetRparamCSP, uint8_t, Node);
    UT_GenStub_AddParam(CFE_SRL_ApiSetRparamCSP, uint8_t, TableId);
    UT_GenStub_AddParam(CFE_SRL_ApiSetRparamCSP, uint16_t, Addr);
    UT_GenStub_AddParam(CFE_SRL_ApiSetRparamCSP, void *, Param);

    UT_GenStub_Execute(CFE_SRL_ApiSetRparamCSP, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiSetRparamCSP, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiTransactionCSP()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiTransactionCSP(uint8_t Node, uint8_t Port, void *TxData, int TxSize, void *RxData, int RxSize)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiTransactionCSP, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiTransactionCSP, uint8_t, Node);
    UT_GenStub_AddParam(CFE_SRL_ApiTransactionCSP, uint8_t, Port);
    UT_GenStub_AddParam(CFE_SRL_ApiTransactionCSP, void *, TxData);
    UT_GenStub_AddParam(CFE_SRL_ApiTransactionCSP, int, TxSize);
    UT_GenStub_AddParam(CFE_SRL_ApiTransactionCSP, void *, RxData);
    UT_GenStub_AddParam(CFE_SRL_ApiTransactionCSP, int, RxSize);

    UT_GenStub_Execute(CFE_SRL_ApiTransactionCSP, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiTransactionCSP, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_ApiWrite()
 * ----------------------------------------------------
 */
int32 CFE_SRL_ApiWrite(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_ApiWrite, int32);

    UT_GenStub_AddParam(CFE_SRL_ApiWrite, CFE_SRL_IO_Handle_t *, Handle);
    UT_GenStub_AddParam(CFE_SRL_ApiWrite, CFE_SRL_IO_Param_t *, Params);

    UT_GenStub_Execute(CFE_SRL_ApiWrite, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_ApiWrite, int32);
}
