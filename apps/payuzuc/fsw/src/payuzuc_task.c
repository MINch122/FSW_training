/**
 * \file
 *   This file contains the source code for the Payload UZURO Cam App.
 */

/*
** Include Files:
*/
#include "payuzuc_task.h"
#include "payuzuc_cmds.h"
#include "payuzuc_eventids.h"
#include "payuzuc_dispatch.h"
#include "payuzuc_utils.h"

/**
 * Global Data
 */
PAYUZUC_Data_t PAYUZUC_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void PAYUZUC_Main(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(PAYUZUC_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    Status = PAYUZUC_Init();

    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /**
     * PAYUZUC Task Main Runloop
     */
    while (CFE_ES_RunLoop(&PAYUZUC_Data.RunStatus) == true) {
        /**
         * Performance Log Exit stamp
         */
        CFE_ES_PerfLogExit(PAYUZUC_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYUZUC_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /**
         * Performance log Entry Stamp
         */
        CFE_ES_PerfLogEntry(PAYUZUC_PERF_ID);

        if (Status == CFE_SUCCESS) {
            PAYUZUC_TaskPipe(SBBufPtr);
        }
        else {
            CFE_EVS_SendEvent(PAYUZUC_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "PAYUZUC: SB Pipe Read Error, App will Exit");
            PAYUZUC_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }
    Status = PAYUZUC_CloseFile(PAYUZUC_Data.TblHandle);
    if (Status != 0) OS_printf("Close Fail App close.\n");

    /**
     * Performance Log Exit Stamp
     */
    CFE_ES_PerfLogExit(PAYUZUC_PERF_ID);

    CFE_ES_ExitApp(PAYUZUC_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_Init(void) {
    CFE_Status_t Status;

    memset(&PAYUZUC_Data, 0, sizeof(PAYUZUC_Data));

    PAYUZUC_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /**
     * Initialize app configuration data
     */
    PAYUZUC_Data.PipeDepth = PAYUZUC_PIPE_DEPTH;

    strncpy(PAYUZUC_Data.PipeName, "PAYUZUC_CMD_PIPE", sizeof(PAYUZUC_Data.PipeName));
    PAYUZUC_Data.PipeName[sizeof(PAYUZUC_Data.PipeName)-1] = 0;

    /**
     * Register Events
     */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Error Registering Events, RC = 0x%08lX\n", __func__, (unsigned long)Status);
    }
    else {
        /**
         * Initialize housekeeping packet (clear user data area)
         */
        CFE_MSG_Init(CFE_MSG_PTR(PAYUZUC_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUC_BCN_TLM_MID),
                        sizeof(PAYUZUC_Data.BcnTlm));

        /**
         * Create SB message pipe
         */
        Status = CFE_SB_CreatePipe(&PAYUZUC_Data.CommandPipe, PAYUZUC_Data.PipeDepth, PAYUZUC_Data.PipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUC_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error creating SB Command Pipe, RC= 0x%08lX", __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to housekeeping request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUZUC_SEND_HK_MID), PAYUZUC_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUC_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to HK request, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to beacon request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUZUC_SEND_BCN_MID), PAYUZUC_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUC_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to HK request, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUZUC_CMD_MID), PAYUZUC_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUC_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to Commands, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAYUZUC_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "%s: PAYUZUC Initialized", __func__);
    }

    /**
     * Get Serial Handle pointer
     */
    PAYUZUC_Data.Handle = CFE_SRL_ApiGetHandle(CFE_SRL_SOCAT_HANDLE_INDEXER);
    CFE_ES_WriteToSysLog("%s: IO Handle Ptr: %p", __func__, (void *)PAYUZUC_Data.Handle);

    /**
     * Get PAYUZUC Tbl Handle
     */
    PAYUZUC_Data.TblHandle = PAYUZUC_OpenTblFile();
    if (PAYUZUC_Data.TblHandle < 0) {
        OS_printf("PAYUZUC Table Open Fail.\n");
        Status = -1;
        return Status;
    }
    Status = PAYUZUC_ReadFile(PAYUZUC_Data.TblHandle, &PAYUZUC_Data.MemSlotStatus, sizeof(PAYUZUC_Memory_Status_t));
    if (Status < 0) {
        OS_printf("Read Error.\n");
    }
    else if (Status == sizeof(PAYUZUC_Memory_Status_t)) Status = CFE_SUCCESS;

    OS_MutSemCreate(&PAYUZUC_Data.MutId, PAYUZUC_MUTEX_NAME, 0);

    return Status;
}