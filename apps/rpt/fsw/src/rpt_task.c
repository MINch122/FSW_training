/**
 * \file
 *   This file contains the source code for the RPT App.
 */

/**
 * Include Files
 */
#include "rpt_task.h"
#include "../inc/rpt_eventids.h"
#include "rpt_dispatch.h"
#include "rpt_utils.h"
#include "cfe_msgids.h"



/**
 * Global data
 */
RPT_Data_t RPT_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void RPT_Main(void) {
    CFE_Status_t Status;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(RPT_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    Status = RPT_Init();
    if (Status != CFE_SUCCESS) {
        RPT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /**
     * RPT Runloop
     */
    while (CFE_ES_RunLoop(&RPT_Data.RunStatus) == true) {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(RPT_PERF_ID);

        OS_TaskDelay(50);

        CFE_ES_PerfLogEntry(RPT_PERF_ID);

        /**
         * Each Task correspond Pipe
         */
        RPT_ForwardReport();
        RPT_FowardCritical();
        
        RPT_ForwardCommand();

    }

    /**
     * Performance Log Exit Stamp
     */
    CFE_ES_PerfLogExit(RPT_PERF_ID);

    CFE_ES_ExitApp(RPT_Data.RunStatus);

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Task for each SB pipe                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void RPT_ForwardReport(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    for(;;) {
        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, RPT_Data.RptPipe, CFE_SB_POLL);
        if (Status != CFE_SUCCESS) {
            break;
        }

        /**
         * Report Handling function
         */
        if (RPT_VerifyReportLength(&SBBufPtr->Msg)) {
            RPT_Report(&((const RPT_ReportTlm_t *)SBBufPtr)->Payload, false);
        }
    }
}

void RPT_FowardCritical(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    for (;;) {
        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, RPT_Data.CritPipe, CFE_SB_POLL);
        if (Status != CFE_SUCCESS) {
            break;
        }

        /**
         * Critical Handling function
         */
        if (RPT_VerifyReportLength(&SBBufPtr->Msg)) {
            RPT_Report(&((const RPT_ReportTlm_t *)SBBufPtr)->Payload, true);
        }

    }
}

void RPT_ForwardCommand(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    for (;;) {
        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, RPT_Data.CmdPipe, CFE_SB_POLL);
        if (Status != CFE_SUCCESS) {
            break;
        }

        /**
         * Task Pipe function
         */
        RPT_TaskPipe(SBBufPtr);
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* RPT Initialization                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t RPT_Init(void) {
    CFE_Status_t Status;
    int32 OsStatus;
    void *TempTblPtr;

    memset(&RPT_Data, 0, sizeof(RPT_Data));

    RPT_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /**
     * Initialize app configuration data
     */
    RPT_Data.PipeDepth = RPT_PIPE_DEPTH;

    strncpy(RPT_Data.CmdPipeName, "RPT_CMD_PIPE", sizeof(RPT_Data.CmdPipeName));
    RPT_Data.CmdPipeName[sizeof(RPT_Data.CmdPipeName) - 1] = 0;

    strncpy(RPT_Data.RptPipeName, "RPT_RPT_PIPE", sizeof(RPT_Data.RptPipeName));
    RPT_Data.RptPipeName[sizeof(RPT_Data.RptPipeName - 1)] = 0;

    strncpy(RPT_Data.CritPipeName, "RPT_CRITICAL_PIPE", sizeof(RPT_Data.CritPipeName));
    RPT_Data.CritPipeName[sizeof(RPT_Data.CritPipeName - 1)] = 0;

    /**
     * Register the events
     */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) { 
        CFE_ES_WriteToSysLog("RPT: Error Registering Events. RC = 0x%08lX\n", (unsigned long)Status);
    }
    else {
        /**
         * Initialize housekeeping packet
         */
        CFE_MSG_Init(CFE_MSG_PTR(RPT_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(RPT_BCN_TLM_MID),
                        sizeof(RPT_Data.HkTlm));
        /**
         * Create Software Bus Message Pipe.
         */
        Status = CFE_SB_CreatePipe(&RPT_Data.CmdPipe, RPT_Data.PipeDepth, RPT_Data.CmdPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to Housekeeping request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RPT_SEND_HK_MID), RPT_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)Status);
        }

    }
    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to Beacon request commands
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RPT_SEND_BCN_MID), RPT_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)Status);
        }
    }
    
    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to `CFE_TIME_ONEHZ_CMD_MID`
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CFE_TIME_ONEHZ_CMD_MID), RPT_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_SUB_ONEHZ_ERR_EID, CFE_EVS_EventType_ERROR,
                                "RPT: Error Subscribing to ONEHZ MID, RC = 0x%08lX", (unsigned long)Status);
        }
    }
    
    if (Status == CFE_SUCCESS) {
        /**
         * Subscribe to ground command packets
         */
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RPT_CMD_MID), RPT_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error Subscribing to Cmd request, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    /*************************************
     * 
     * Table Init
     * 
     *************************************/
    Status = CFE_TBL_Register(&RPT_Data.SubsTblHandle, "RPT_Subs", (sizeof(RPT_Table_t) * RPT_MAX_TBL_ENTRY),
                        CFE_TBL_OPT_DEFAULT, NULL);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d RPT Can't register table. RC = %d",
                            __LINE__, Status);
    }
    else {
        Status = CFE_TBL_Load(RPT_Data.SubsTblHandle, CFE_TBL_SRC_FILE, RPT_TABLE_FILE);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d RPT Can't load table. RC = %d",
                                __LINE__, Status);
        }
    }
    if (Status == CFE_SUCCESS) {
        Status = CFE_TBL_GetAddress((void **)&TempTblPtr, RPT_Data.SubsTblHandle);
        if(Status != CFE_SUCCESS && Status != CFE_TBL_INFO_UPDATED) {
            CFE_EVS_SendEvent(RPT_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d RPT Can't get table addr. RC = %d",
                                __LINE__, Status);
        }
    }
    if (Status == CFE_SUCCESS || Status == CFE_TBL_INFO_UPDATED) {
        RPT_Data.SubsTblPtr = TempTblPtr; /* Save returned address */

        /**
         * Table init success, then create pipe
         */
        Status = CFE_SB_CreatePipe(&RPT_Data.RptPipe, RPT_Data.PipeDepth, RPT_Data.RptPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error creating SB Report Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
        Status = CFE_SB_CreatePipe(&RPT_Data.CritPipe, RPT_Data.PipeDepth, RPT_Data.CritPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error creating SB Critical Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        /* Subscription */
        for (uint8_t i = 0; i < RPT_MAX_TBL_ENTRY; i ++) {
            if (RPT_Data.SubsTblPtr->UsedState == RPT_DISABLED) continue;

            if (CFE_SB_IsValidMsgId(RPT_Data.SubsTblPtr->Entry.MessageID) == false) continue;

            /**
             * Subscribe each entry
             */
            if (RPT_Data.SubsTblPtr->Entry.IsCritical) {
                Status = CFE_SB_SubscribeEx(RPT_Data.SubsTblPtr->Entry.MessageID, 
                                            RPT_Data.CritPipe, (CFE_SB_Qos_t){0, 0}, RPT_CRITICAL_MSG_DEPTH);
            }
            else {
                Status = CFE_SB_SubscribeEx(RPT_Data.SubsTblPtr->Entry.MessageID,
                                            RPT_Data.RptPipe, (CFE_SB_Qos_t){0, 0}, RPT_REPORT_MSG_DEPTH);
            }

            if (Status != CFE_SUCCESS) {
                CFE_EVS_SendEvent(RPT_REPORT_SUB_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "L%d RPT Can't subscribe to stream 0x%X status %i", __LINE__,
                                  (unsigned int)CFE_SB_MsgIdToValue(RPT_Data.SubsTblPtr->Entry.MessageID), (int)Status);
            }
            RPT_Data.SubsTblPtr ++;
        }
    }
    /***********************************
     * 
     * Mutex Init
     * 
     **********************************/
    
    OsStatus = OS_MutSemCreate(&RPT_Data.ReportMutexID, "RPT_Report_Mutex", 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendEvent(RPT_MUTEX_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RPT Report Queue Mutex create failed. OS status = %d\n", OsStatus);
        return OsStatus;
    }
    OsStatus = OS_MutSemCreate(&RPT_Data.CritMutexID, "RPT_Critical_Mutex", 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendEvent(RPT_MUTEX_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RPT Critical Queue Mutex create failed. OS status = %d\n", OsStatus);
        return OsStatus;
    }
    OsStatus = OS_MutSemCreate(&RPT_Data.OpsMutexID, "RPT_OPS_Mutex", 0);
    if (OsStatus != OS_SUCCESS) {
        CFE_EVS_SendEvent(RPT_MUTEX_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RPT Ops Mutex create failed. OS status = %d\n", OsStatus);
        return OsStatus;
    }

    /***********************************
     * 
     * Queue Init
     * 
     **********************************/
    Status = RPT_PriorInit();
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_PRIOR_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RPT Prior Init failed. RC = %d", Status);
    }

    /********************************
     * 
     * Ops Data Init
     * 
     *******************************/
    Status = RPT_OpsDataInit();
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_OPS_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RPT Operation data init failed. RC = %d", Status);
    }

    /********************************
     * 
     * Critical Data Init
     * 
     *******************************/
    if (Status == CFE_SUCCESS) {
        Status = RPT_CriticalQInit();
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_CRIT_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RPT Critical data init failed. RC = %d", Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "RPT Initialization success.");
    }

    return Status;
}

CFE_Status_t RPT_OpsDataInit(void) {
    CFE_Status_t Status;
    
    RPT_Data.OpsDataHandle = RPT_OpenOpsFile(false);
    if (RPT_Data.OpsDataHandle == -1) {
        OS_printf("RPT Ops Open failed.\n");
    }

    Status = RPT_ReadFromFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
    if (Status < 0) {
        OS_printf("RPT Ops Read Error.\n");
    }
    else if (RPT_Data.OpsData.BootCount == 0 || Status == 0) Status = CFE_SUCCESS;
    else if (RPT_Data.OpsData.BootCount != 0 || Status == sizeof(RPT_OperationData_t)) Status = CFE_SUCCESS;
    else Status = -1; // Revise

    
    if (Status == CFE_SUCCESS) {
        /**
         * CRC Check
         */
        uint32 CRC = RPT_CalculateCRC(&RPT_Data.OpsData, (sizeof(RPT_OperationData_t) - sizeof(uint32_t)));
        if (RPT_Data.OpsData.CRC == CRC) {
            OS_printf("Ops CRC well matched.\n");
            Status = CFE_SUCCESS;
        }
        else Status = -1; // Revise
    }

    if (Status == CFE_SUCCESS) {
        /**
         * If successfully read ops data, then increase the boot count.
         */
        RPT_Data.OpsData.BootCount ++;
        OS_printf("Boot Count: %u\n", RPT_Data.OpsData.BootCount);
        
        RPT_Data.OpsData.CRC = RPT_CalculateCRC(&RPT_Data.OpsData, (sizeof(RPT_OperationData_t) - sizeof(uint32_t)));
        Status = RPT_WriteToFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
        if (Status != CFE_SUCCESS) {
            OS_printf("RPT Ops write error.\n");
            Status = -1;
        }
    }

    return Status;
}

CFE_Status_t RPT_CriticalQInit(void) {
    CFE_Status_t Status;

    RPT_Data.CritDataHandle = RPT_OpenCriticalFile();
    if (RPT_Data.CritDataHandle == -1) {
        OS_printf("RPT Critical Open failed.\n");
    }

    Status = RPT_ReadFromFile(RPT_Data.CritDataHandle, &RPT_Data.CritQueue, sizeof(RPT_CriticalQueue_t));
    if (Status < 0) {
        OS_printf("RPT critical Read fail.\n");
    }
    else if (RPT_Data.OpsData.BootCount == 0 || Status == 0) Status = CFE_SUCCESS;
    else if (RPT_Data.OpsData.BootCount != 0 || Status == sizeof(RPT_CriticalQueue_t)) Status = CFE_SUCCESS;

    if (Status == CFE_SUCCESS) {
        /**
         * CRC Check
         */
        uint32 CRC = RPT_CalculateCRC(&RPT_Data.CritQueue, (sizeof(RPT_CriticalQueue_t) - sizeof(uint32_t)));
        if (RPT_Data.CritQueue.CRC == CRC) {
            OS_printf("Critical CRC well matched.\n");
            Status = CFE_SUCCESS;
        }
        else {
            OS_printf("Critical CRC not matched. Clear Critical Queue.\n");
            memset(&RPT_Data.CritQueue, 0, sizeof(RPT_CriticalQueue_t));
            Status = CFE_SUCCESS;
        }
    }

    return Status;
}