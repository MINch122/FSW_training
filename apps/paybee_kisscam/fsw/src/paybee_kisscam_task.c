/**
 * \file
 *   This file contains the source code for the Payload UZURO Cam App.
 */

/*
** Include Files:
*/
#include "paybee_kisscam_task.h"
#include "paybee_kisscam_cmds.h"
#include "paybee_kisscam_eventids.h"
#include "paybee_kisscam_dispatch.h"
#include "paybee_kisscam_utils.h"

/**
 * Global Data
 */
paybee_kisscam_Data_t paybee_kisscam_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void paybee_kisscam_Main(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(paybee_kisscam_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    Status = paybee_kisscam_Init();

    if (Status != CFE_SUCCESS) {
        paybee_kisscam_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /**
     * paybee_kisscam Task Main Runloop
     */
    while (CFE_ES_RunLoop(&paybee_kisscam_Data.RunStatus) == true) {
        /**
         * Performance Log Exit stamp
         */
        CFE_ES_PerfLogExit(paybee_kisscam_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, paybee_kisscam_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /**
         * Performance log Entry Stamp
         */
        CFE_ES_PerfLogEntry(paybee_kisscam_PERF_ID);

        if (Status == CFE_SUCCESS) {
            paybee_kisscam_TaskPipe(SBBufPtr);
        }
        else {
            CFE_EVS_SendEvent(paybee_kisscam_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "paybee_kisscam: SB Pipe Read Error, App will Exit");
            paybee_kisscam_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }
    Status = paybee_kisscam_CloseFile(paybee_kisscam_Data.TblHandle);
    // if (Status != 0) PAYBEE_KISSCAM_APP_printf("Close Fail App close.\n");

    /**
     * Performance Log Exit Stamp
     */
    CFE_ES_PerfLogExit(paybee_kisscam_PERF_ID);

    CFE_ES_ExitApp(paybee_kisscam_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_Init(void) {
    CFE_Status_t Status;

    memset(&paybee_kisscam_Data, 0, sizeof(paybee_kisscam_Data));

    paybee_kisscam_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /**
     * Initialize app configuration data
     */
    paybee_kisscam_Data.PipeDepth = paybee_kisscam_PIPE_DEPTH;

    strncpy(paybee_kisscam_Data.PipeName, "paybee_kisscam_CMD_PIPE", sizeof(paybee_kisscam_Data.PipeName));
    paybee_kisscam_Data.PipeName[sizeof(paybee_kisscam_Data.PipeName)-1] = 0;

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
        // CFE_MSG_Init(CFE_MSG_PTR(paybee_kisscam_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(paybee_kisscam_BCN_TLM_MID),
        //                 sizeof(paybee_kisscam_Data.BcnTlm));

        /**
         * Create SB message pipe
         */
        Status = CFE_SB_CreatePipe(&paybee_kisscam_Data.CommandPipe, paybee_kisscam_Data.PipeDepth, paybee_kisscam_Data.PipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(paybee_kisscam_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error creating SB Command Pipe, RC= 0x%08lX", __func__, (unsigned long)Status);
        }
    }

    // if (Status == CFE_SUCCESS) {
    //     /**
    //      * Subscribe to housekeeping request commands
    //      */
    //     Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(paybee_kisscam_SEND_HK_MID), paybee_kisscam_Data.CommandPipe);
    //     if (Status != CFE_SUCCESS) {
    //         CFE_EVS_SendEvent(paybee_kisscam_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
    //                             "%s: Error Subscribing to HK request, RC = 0x%08lX", 
    //                             __func__, (unsigned long)Status);
    //     }
    // }

    // if (Status == CFE_SUCCESS) {
    //     /**
    //      * Subscribe to beacon request commands
    //      */
    //     Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(paybee_kisscam_SEND_BCN_MID), paybee_kisscam_Data.CommandPipe);
    //     if (Status != CFE_SUCCESS) {
    //         CFE_EVS_SendEvent(paybee_kisscam_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
    //                             "%s: Error Subscribing to HK request, RC = 0x%08lX", 
    //                             __func__, (unsigned long)Status);
    //     }
    // }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(paybee_kisscam_CMD_MID), paybee_kisscam_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(paybee_kisscam_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to Commands, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(paybee_kisscam_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "%s: paybee_kisscam Initialized", __func__);
    }

    /**
     * Get Serial Handle pointer
     */
    paybee_kisscam_Data.Handle = CFE_SRL_ApiGetHandle(CFE_SRL_RS422_HANDLE_INDEXER);
    // CFE_ES_WriteToSysLog("%s: IO Handle Ptr: %p", __func__, (void *)paybee_kisscam_Data.Handle);

    /**
     * Get paybee_kisscam Tbl Handle
     */
    paybee_kisscam_Data.TblHandle = paybee_kisscam_OpenTblFile();
    if (paybee_kisscam_Data.TblHandle < 0) {
        // PAYBEE_KISSCAM_APP_printf("paybee_kisscam Table Open Fail.\n");
        Status = -1;
        return Status;
    }
    Status = paybee_kisscam_ReadFile(paybee_kisscam_Data.TblHandle, &paybee_kisscam_Data.MemSlotStatus, sizeof(paybee_kisscam_Memory_Status_t));
    if (Status < 0) {
        // PAYBEE_KISSCAM_APP_printf("Read Error.\n");
    }
    else if (Status == sizeof(paybee_kisscam_Memory_Status_t)) Status = CFE_SUCCESS;

    // OS_MutSemCreate(&paybee_kisscam_Data.MutId, paybee_kisscam_MUTEX_NAME, 0);

    return Status;
}