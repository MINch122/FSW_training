/**
 * \file
 *   This file contains the source code for the FTP App.
 */

/*
** Include Files:
*/
#include "ftp_task.h"
#include "ftp_cmds.h"
#include "ftp_eventids.h"
#include "ftp_dispatch.h"
#include "ftp_utils.h"

/**
 * Global data
 */
FTP_Data_t FTP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void FTP_Main(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(FTP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    Status = FTP_Init();

    if (Status != CFE_SUCCESS) {
        FTP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /**
     * FTPUZUC Task Main Runloop
     */
    while (CFE_ES_RunLoop(&FTP_Data.RunStatus) == true) {
        /**
         * Performance Log Exit stamp
         */
        CFE_ES_PerfLogExit(FTP_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, FTP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /**
         * Performance log Entry Stamp
         */
        CFE_ES_PerfLogEntry(FTP_PERF_ID);

        if (Status == CFE_SUCCESS) {
            FTP_TaskPipe(SBBufPtr);
        }
        else {
            CFE_EVS_SendEvent(FTP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "FTP: SB Pipe Read Error, App will Exit");
            FTP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /**
     * Performance Log Exit Stamp
     */
    CFE_ES_PerfLogExit(FTP_PERF_ID);

    CFE_ES_ExitApp(FTP_Data.RunStatus);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t FTP_Init(void) {
    CFE_Status_t Status;
    int32 OsStatus;
    osal_id_t TimeBaseId = OS_OBJECT_ID_UNDEFINED;

    memset(&FTP_Data, 0, sizeof(FTP_Data));

    FTP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /**
     * Initialize app configuration data
     */
    FTP_Data.PipeDepth = FTP_PIPE_DEPTH;

    strncpy(FTP_Data.PipeName, "FTP_CMD_PIPE", sizeof(FTP_Data.PipeName));
    FTP_Data.PipeName[sizeof(FTP_Data.PipeName)-1] = 0;

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
        CFE_MSG_Init(CFE_MSG_PTR(FTP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(FTP_HK_TLM_MID),
                    sizeof(FTP_Data.HkTlm));

        /**
         * Initializae FTP file packet
         */
        CFE_MSG_Init(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), CFE_SB_ValueToMsgId(FTP_FILE_MID),
                    sizeof(FTP_Data.Chunk));

        /**
         * Initializae FTP Report tlm packet
         */
        CFE_MSG_Init(CFE_MSG_PTR(FTP_Data.Report.TelemetryHeader), CFE_SB_ValueToMsgId(FTP_REPORT_TLM_MID),
                    sizeof(FTP_Data.Report));

        /**
         * Create SB message pipe
         */
        Status = CFE_SB_CreatePipe(&FTP_Data.CommandPipe, FTP_Data.PipeDepth, FTP_Data.PipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(FTP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error creating SB Command Pipe, RC= 0x%08lX", __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to housekeeping request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(FTP_SEND_HK_MID), FTP_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(FTP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to HK request, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to beacon request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(FTP_SEND_BCN_MID), FTP_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(FTP_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to Commands, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(FTP_CMD_MID), FTP_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(FTP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to Commands, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    /* The underlying timebase object should have been created by the PSP */
    OsStatus = OS_TimeBaseGetIdByName(&TimeBaseId, "cFS-Master");
    if (OsStatus != OS_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: OS_TimeBaseGetIdByName failed:RC=%ld\n", __func__, (long)OsStatus);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Create the timer callback (but not set yet, as that requires the config table) */
    OS_TimerAdd(&FTP_Data.TimerId, "FTP_APP", TimeBaseId, FTP_LocalTimerCallback, NULL);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_ES_WriteToSysLog("%s: OS_TimerAdd failed:RC=%ld\n", __func__, (long)OsStatus);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (Status == CFE_SUCCESS && OsStatus == OS_SUCCESS) {
        CFE_EVS_SendInfo(FTP_INIT_INF_EID, "FTP App successfully Initialized.");
    }

    return Status;
}