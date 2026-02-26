/**
 * \file
 *   This file contains the source code for the EO App.
 */

/**
 * Include Files
 */
#include "eo_task.h"
#include "eo_eventids.h"
#include "eo_dispatch.h"
#include "eo_utils.h"
#include "cfe_msgids.h"
#include "eo_file.h"
#include "eo_child.h"


/**
 * Global data
 */
EO_Data_t EO_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void EO_Main(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(EO_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    Status = EO_Init();
    if (Status != CFE_SUCCESS) {
        EO_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /**
     * EO Runloop
     */
    while (CFE_ES_RunLoop(&EO_Data.RunStatus) == true) {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(EO_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, EO_Data.CmdPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(EO_PERF_ID);

        if (Status == CFE_SUCCESS) {
            EO_TaskPipe(SBBufPtr);
        }
        else {
            CFE_EVS_SendEvent(EO_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EO: SB Pipe Read Error, App will Exit");
            EO_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /**
     * Performance Log Exit Stamp
     */
    CFE_ES_PerfLogExit(EO_PERF_ID);

    CFE_ES_ExitApp(EO_Data.RunStatus);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* EO Initialization                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t EO_Init(void) {
    CFE_Status_t Status;
    int32 OsStatus;

    memset(&EO_Data, 0, sizeof(EO_Data));

    EO_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /**
     * Initialize app configuration data
     */
    EO_Data.PipeDepth = EO_PIPE_DEPTH;

    strncpy(EO_Data.CmdPipeName, "EO_CMD_PIPE", sizeof(EO_Data.CmdPipeName));
    EO_Data.CmdPipeName[sizeof(EO_Data.CmdPipeName) - 1] = 0;

    /**
     * Register the events
     */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) { 
        CFE_ES_WriteToSysLog("EO: Error Registering Events. RC = 0x%08lX\n", (unsigned long)Status);
    }
    else {
        /**
         * Initialize housekeeping packet
         */
        CFE_MSG_Init(CFE_MSG_PTR(EO_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(EO_BCN_TLM_MID),
                        sizeof(EO_Data.BcnTlm));
        /**
         * Create Software Bus Message Pipe.
         */
        Status = CFE_SB_CreatePipe(&EO_Data.CmdPipe, EO_Data.PipeDepth, EO_Data.CmdPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(EO_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EO: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to Beacon request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(EO_SEND_BCN_MID), EO_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(EO_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EO: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)Status);
        }
    }
    
    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(EO_CMD_MID), EO_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(EO_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EO: Error Subscribing to Cmd request, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        // TODO: Update EO to use P80 EPS telemetry types
        // Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(EPS_VI_TLM_MID), EO_Data.CmdPipe);
        // Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(EPS_OUT_TLM_MID), EO_Data.CmdPipe);
        (void)0; // placeholder
    }

    // if (Status == CFE_SUCCESS) {
    //     /**
    //      * Subscribe to ground command packets
    //      */
    //     Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SANT_OP_TLM_MID), EO_Data.CmdPipe);
    //     if (Status != CFE_SUCCESS) {
    //         CFE_EVS_SendEvent(EO_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
    //                           "EO: Error Subscribing to SANT Op Tlm, RC = 0x%08lX", (unsigned long)Status);
    //     }
    // }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RPT_OPS_TLM_MID), EO_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(EO_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EO: Error Subscribing to RPT Ops Tlm, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCS_MMT_TLM_MID), EO_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(EO_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EO: Error Subscribing to RPT Ops Tlm, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    /********************************
     * 
     * Current Early Orbit State Init
     * 
     *******************************/
    if (Status == CFE_SUCCESS) {
        Status = EO_StepInit();
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(EO_PHASE_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EO Operation data init failed. RC = %d", Status);
        }
    }

    /* Semaphore creation for synchronization */
    OsStatus = OS_BinSemCreate(&EO_Data.EPS_ViSemId, EO_EPS_VI_SEM, 0, 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendErr(EO_SEM_INIT_ERR_EID, "EO EPS Vi Sem Create Err. RC = %d\n", OsStatus);
    }
    OsStatus = OS_BinSemCreate(&EO_Data.EPS_OutSemId, EO_EPS_OUT_SEM, 0, 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendErr(EO_SEM_INIT_ERR_EID, "EO EPS Out Sem Create Err. RC = %d\n", OsStatus);
    }
    OsStatus = OS_BinSemCreate(&EO_Data.SANT_SemId, EO_SANT_SEM, 0, 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendErr(EO_SEM_INIT_ERR_EID, "EO SANT Sem Create Err. RC = %d\n", OsStatus);
    }
    OsStatus = OS_BinSemCreate(&EO_Data.ADCS_SemId, EO_ADCS_SEM, 0, 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendErr(EO_SEM_INIT_ERR_EID, "EO ADCS Sem Create Err. RC = %d\n", OsStatus);
    }

    /* Mutex creation for mutual exclusion */
    OsStatus = OS_MutSemCreate(&EO_Data.EOMutex, EO_PHASE_MUT, 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendErr(EO_SEM_INIT_ERR_EID, "EO Phase Mut Create Err. RC = %d\n", OsStatus);
    }

    if (Status == CFE_SUCCESS && OsStatus == OS_SUCCESS) {
        CFE_EVS_SendEvent(EO_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "EO app Successfully Initialized.\n");
    }

    return Status;
}