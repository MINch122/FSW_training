/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
 *
 * Licensed under the Apache License, Version 2.0
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Thrust App.
 */

/*
** Include Files:
*/
#include "thrust_app.h"
#include "thrust_cmds.h"
#include "thrust_cmds_list.h"
#include "thrust_dispatch.h"
#include "thrust_eventids.h"

/*
** Global Data
*/
THRUST_AppData_t THRUST_AppData;
CFE_SRL_IO_Handle_t *Handle;

static int32_t THRUST_TransportWrapper(const void *tx_buf,
                                       uint16_t    tx_len,
                                       void       *rx_buf,
                                       int32_t     rx_size,
                                       uint16_t    timeout_ms)
{
    CFE_SRL_IO_Param_t Params = {0};

    Params.TxData  = (void *)tx_buf;
    Params.TxSize  = tx_len;
    Params.RxData  = (void *)rx_buf;
    Params.RxSize  = rx_size;
    Params.Timeout = timeout_ms;

    if (tx_len > 0 && rx_size == 0)
    {
        return CFE_SRL_ApiWrite(Handle, &Params);
    }

    if (rx_size > 0 && tx_len == 0)
    {
        return CFE_SRL_ApiRead(Handle, &Params);
    }

    return THRUST_HAL_ERROR;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void THRUST_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(THRUST_PERF_ID);

    status = THRUST_AppInit();
    if (status != CFE_SUCCESS)
    {
        THRUST_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&THRUST_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(THRUST_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, THRUST_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(THRUST_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            THRUST_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(THRUST_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "THRUST APP: SB Pipe Read Error, App Will Exit");
            THRUST_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(THRUST_PERF_ID);
    CFE_ES_ExitApp(THRUST_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t THRUST_AppInit(void)
{
    CFE_Status_t status;

    /* Zero out the global data structure */
    memset(&THRUST_AppData, 0, sizeof(THRUST_AppData));

    THRUST_AppData.RunStatus       = CFE_ES_RunStatus_APP_RUN;
    THRUST_AppData.PipeDepth       = THRUST_PIPE_DEPTH;
    THRUST_AppData.ProtocolVersion = THRUST_PKT_VERSION;

    strncpy(THRUST_AppData.PipeName, "THRUST_CMD_PIPE", sizeof(THRUST_AppData.PipeName));
    THRUST_AppData.PipeName[sizeof(THRUST_AppData.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Thrust App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Initialize housekeeping packet.
    */
    CFE_MSG_Init(CFE_MSG_PTR(THRUST_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(THRUST_HK_TLM_MID),
                 sizeof(THRUST_AppData.HkTlm));

    /*
    ** Initialize status packet.
    */
    CFE_MSG_Init(CFE_MSG_PTR(THRUST_AppData.StatusTlm.TelemetryHeader), CFE_SB_ValueToMsgId(THRUST_STATUS_TLM_MID),
                 sizeof(THRUST_AppData.StatusTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&THRUST_AppData.CommandPipe, THRUST_AppData.PipeDepth, THRUST_AppData.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(THRUST_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THRUST App: Error creating SB pipe, RC = 0x%08lX",
                          (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to Housekeeping request commands.
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(THRUST_SEND_HK_MID), THRUST_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(THRUST_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THRUST App: Error subscribing to HK request, RC = 0x%08lX",
                          (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to ground command packets.
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(THRUST_CMD_MID), THRUST_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(THRUST_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THRUST App: Error subscribing to commands, RC = 0x%08lX",
                          (unsigned long)status);
        return status;
    }

    /*
    ** Register RS-422 transport.
    */
    Handle = CFE_SRL_ApiGetHandle(CFE_SRL_RS422_HANDLE_INDEXER);
    THRUST_RegisterTransport(THRUST_TransportWrapper);

    CFE_EVS_SendEvent(THRUST_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "THRUST App Initialized.");

    return CFE_SUCCESS;
}
