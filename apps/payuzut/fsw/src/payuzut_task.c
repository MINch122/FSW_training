/**
 * \file
 *   This file contains the source code for the Payload UZURO Cam App.
 */

/*
** Include Files:
*/
#include "payuzut_task.h"
#include "payuzut_cmds.h"
#include "payuzut_eventids.h"
#include "payuzut_dispatch.h"
#include "payuzut_utils.h"

/**
 * Global Data
 */
PAYUZUT_Data_t PAYUZUT_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void PAYUZUT_Main(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(PAYUZUT_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    Status = PAYUZUT_Init();

    if (Status != CFE_SUCCESS) {
        PAYUZUT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /**
     * PAYUZUT Task Main Runloop
     */
    while (CFE_ES_RunLoop(&PAYUZUT_Data.RunStatus) == true) {
        /**
         * Performance Log Exit stamp
         */
        CFE_ES_PerfLogExit(PAYUZUT_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYUZUT_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /**
         * Performance log Entry Stamp
         */
        CFE_ES_PerfLogEntry(PAYUZUT_PERF_ID);

        if (Status == CFE_SUCCESS) {
            PAYUZUT_TaskPipe(SBBufPtr);
        }
        else {
            CFE_EVS_SendEvent(PAYUZUT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "PAYUZUT: SB Pipe Read Error, App will Exit");
            PAYUZUT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /**
     * Performance Log Exit Stamp
     */
    CFE_ES_PerfLogExit(PAYUZUT_PERF_ID);

    CFE_ES_ExitApp(PAYUZUT_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_Init(void) {
    CFE_Status_t Status;

    memset(&PAYUZUT_Data, 0, sizeof(PAYUZUT_Data));

    PAYUZUT_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /**
     * Initialize app configuration data
     */
    PAYUZUT_Data.PipeDepth = PAYUZUT_PIPE_DEPTH;

    strncpy(PAYUZUT_Data.PipeName, "PAYUZUT_CMD_PIPE", sizeof(PAYUZUT_Data.PipeName));
    PAYUZUT_Data.PipeName[sizeof(PAYUZUT_Data.PipeName)-1] = 0;

    /**
     * Register Events
     */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Error Registering Events, RC = 0x%08lX\n", __func__, (unsigned long)Status);
    }
    else {
        /**
         * Initialize Beacon packet (clear user data area)
         */
        CFE_MSG_Init(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_BCN_TLM_MID),
                        sizeof(PAYUZUT_Data.BcnTlm));

        /**
         * Create SB message pipe
         */
        Status = CFE_SB_CreatePipe(&PAYUZUT_Data.CommandPipe, PAYUZUT_Data.PipeDepth, PAYUZUT_Data.PipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error creating SB Command Pipe, RC= 0x%08lX", __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to Beacon request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUZUT_SEND_BCN_MID), PAYUZUT_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUT_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to HK request, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUZUT_CMD_MID), PAYUZUT_Data.CommandPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAYUZUT_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Error Subscribing to Commands, RC = 0x%08lX", 
                                __func__, (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAYUZUT_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "%s: PAYUZUT Initialized", __func__);
    }

    /**
     * Get Serial Handle pointer
     * I2C1. For temperature
     */
    PAYUZUT_Data.Handle = CFE_SRL_ApiGetHandle(CFE_SRL_I2C1_HANDLE_INDEXER);
    CFE_ES_WriteToSysLog("%s: IO Handle Ptr: %p", __func__, (void *)PAYUZUT_Data.Handle);

    /**
     * Get GPIO Handle Pointer
     * PC5. For thruster
     */
    PAYUZUT_Data.GpioHandle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_THRUSTER_GPIO_INDEXER);
    CFE_ES_WriteToSysLog("%s: GPIO Handle Ptr: %p", __func__, (void *)PAYUZUT_Data.GpioHandle);
    
    return Status;
}