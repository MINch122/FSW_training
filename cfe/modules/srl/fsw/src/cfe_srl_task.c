/******************************************************************************
** File: cfe_sb_task.c
**
** Purpose:
**      This file contains the source code for the SRL task.
**
** Author:   Kweon Hyeok-jin
**
******************************************************************************/

/* Include Files */
#include "cfe_srl_module_all.h"
#include "cfe_srl_task.h"

#include "cfe_version.h"
#include "cfe_config.h"
#include "target_config.h"
#include "cfe_srl_msgids.h"

/**
 * Serial Service (SRL) Task global data;
 */
CFE_SRL_TaskData_t CFE_SRL_TaskData;

extern CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];

void CFE_SRL_TaskMain(void) {
    int32 Status;
    CFE_SB_Buffer_t *SBBufPtr;

    /**
     * Performance Time Stamp Entry
     */
    CFE_ES_PerfLogEntry(CFE_MISSION_SRL_MAIN_PERF_ID);

    /**
     * Perform task specific initialization.
     */
    Status = CFE_SRL_TaskInit();
    if (Status != CFE_SUCCESS) {
        /**
         * Create a syslog entry
         */
        CFE_ES_WriteToSysLog("%s: Application Init Failed,RC=0x%08X\n", __func__, Status);

        /**
         * Allow Core App to Exit
         */
        CFE_ES_ExitApp(CFE_ES_RunStatus_CORE_APP_INIT_ERROR);
    }

    /*
     * Wait for other apps to start.
     * It is important that the core apps are present before this starts receiving
     * messages from the command pipe, as some of those handlers might depend on
     * the other core apps.
     */
    CFE_ES_WaitForSystemState(CFE_ES_SystemState_CORE_READY, CFE_PLATFORM_CORE_MAX_STARTUP_MSEC);

    /**
     * Main Loop
     */
    while (Status == CFE_SUCCESS) {
        /*
        ** Increment the main task execution counter
        **  This is normally done in the CFE_ES_RunLoop call, but
        **  currently CFE Child tasks and the cFE core tasks do not
        **  use the RunLoop call.
        */
        CFE_ES_IncrementTaskCounter();
        OS_printf("SRL TaskMain Loop\n");
        /*
        ** Performance Time Stamp Exit
        */
       CFE_ES_PerfLogExit(CFE_MISSION_SRL_MAIN_PERF_ID);

       /**
        * Wait for the next Software Bus message.
        */
       Status = CFE_SB_ReceiveBuffer(&SBBufPtr, CFE_SRL_TaskData.CmdPipe, CFE_SB_PEND_FOREVER);

       /**
        * Performance Time Stamp Entry
        */
       CFE_ES_PerfLogEntry(CFE_MISSION_SRL_MAIN_PERF_ID);

       if (Status == CFE_SUCCESS) {
        /**
         * Process Message.
         */
        CFE_SRL_TaskPipe(SBBufPtr);
       }
       else {
        CFE_ES_WriteToSysLog("%s: Error reading cmd pipe,RC=0x%08X\n", __func__, Status);
       }
    } /* End Main Loop */

    /* while loop exits only if CFE_SB_ReceiveBuffer returns error */
    CFE_ES_ExitApp(CFE_ES_RunStatus_CORE_APP_RUNTIME_ERROR);
}

int32 CFE_SRL_TaskInit(void) {
    int32 Status;
    char  VersionString[CFE_CFG_MAX_VERSION_STR_LEN];

    /* Get the assigned Application ID for the SB Task */
    CFE_ES_GetAppID(&CFE_SRL_TaskData.AppId);

    /**
     * Register event filter table.
     */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Call to CFE_EVS_Register Failed, RC=0x%08X\n", __func__, Status);
        return Status;
    }

    /**
     * Initialize housekeeping packet
     */
    CFE_MSG_Init(CFE_MSG_PTR(CFE_SRL_TaskData.HKTlmMsg.TelemetryHeader), CFE_SB_ValueToMsgId(CFE_SRL_HK_TLM_MID),
                sizeof(CFE_SRL_TaskData.HKTlmMsg));

    Status = CFE_SB_CreatePipe(&CFE_SRL_TaskData.CmdPipe, CFE_SRL_PIPE_DEPTH, CFE_SRL_PIPE_NAME);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Call to CFE_SB_CreatePipe Failed. RC=0x%08X\n", __func__, Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CFE_SRL_CMD_MID), CFE_SRL_TaskData.CmdPipe);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Subscribe to Cmds Failed. RC=0x%08X\n", __func__, Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CFE_SRL_SEND_HK_MID), CFE_SRL_TaskData.CmdPipe);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Subscribe to Cmds Failed. RC=0x%08X\n", __func__, Status);
        return Status;
    }

    CFE_Config_GetVersionString(VersionString, CFE_CFG_MAX_VERSION_STR_LEN, "cFE",
                                CFE_SRC_VERSION, CFE_BUILD_CODENAME, CFE_LAST_OFFICIAL);

    Status = CFE_EVS_SendEvent(CFE_SRL_TASK_INIT_EID, CFE_EVS_EventType_INFORMATION, "cFE SRL Task Initialized: %s", VersionString);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Error sending init event. RC=0x%08X\n", __func__, Status);
        return Status;
    }

    return CFE_SUCCESS;
}



/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_NoopCmd(const CFE_SRL_NoopCmd_t *Cmd) {
    char VersionString[CFE_CFG_MAX_VERSION_STR_LEN];

    CFE_Config_GetVersionString(VersionString,  CFE_CFG_MAX_VERSION_STR_LEN, "cFE",
                                CFE_SRC_VERSION, CFE_BUILD_CODENAME, CFE_LAST_OFFICIAL);
    
    CFE_SRL_TaskData.HKTlmMsg.Payload.CommandCounter++;

    CFE_EVS_SendEvent(CFE_SRL_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                        "SRL No-op command: %s", VersionString);
    
    return CFE_SUCCESS;
}

int32 CFE_SRL_ResetCounterCmd(const CFE_SRL_ResetCounterCmd_t *Cmd) {

    CFE_SRL_TaskData.HKTlmMsg.Payload.CommandCounter = 0;
    CFE_SRL_TaskData.HKTlmMsg.Payload.CommandErrorCounter = 0;

    CFE_EVS_SendEvent(CFE_SRL_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Reset Counter Cmd Received.");

    return CFE_SUCCESS;
}

int32 CFE_SRL_ResetHandleCounterCmd(const CFE_SRL_ResetHandleCounterCmd_t *Cmd) {
    CFE_SRL_IO_Handle_t *TempHandle = CFE_SRL_ApiGetHandle(Cmd->Payload.Indexer);

    TempHandle->RxCount = 0;
    TempHandle->RxErrCnt = 0;
    TempHandle->TxCount = 0;
    TempHandle->TxErrCnt = 0;

    CFE_EVS_SendEvent(CFE_SRL_RESET_HANDLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Reset Handle Counter Cmd Success.");

    return CFE_SUCCESS;
    
}

/**
 * @deprecated Use `CFE_SRL_SendHkCmd` instead
 */
int32 CFE_SRL_GetHandleStatusCmd(const CFE_SRL_GetHandleStatusCmd_t *Cmd) {

    CFE_EVS_SendEvent(CFE_SRL_GET_HANDLE_STATUS_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Get Handle Status Cmd Received.");

    return CFE_SUCCESS;
}

int32 CFE_SRL_InitHandleCmd(const CFE_SRL_InitHandleCmd_t *Cmd) {
    int32 Status;
    
    Status = CFE_SRL_HandleInit(&Handles[Cmd->Payload.Indexer], Cmd->Payload.Name, Cmd->Payload.DevName, Cmd->Payload.DevType, Cmd->Payload.Indexer, Cmd->Payload.BaudRate, Cmd->Payload.SPIMode);
    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(CFE_SRL_INIT_HANDLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Init Handle Cmd Success.");
    }
    else {
        CFE_StatusString_t String;

        CFE_ES_StatusToString(Status, &String);
        CFE_EVS_SendEvent(CFE_SRL_INIT_HANDLE_INF_EID, CFE_EVS_EventType_ERROR, "SRL Init Handle Cmd failed. RC=%s", String);
    }
    
    return CFE_SUCCESS;
}

int32 CFE_SRL_CloseHandleCmd(const CFE_SRL_CloseHandleCmd_t *Cmd) {
    int32 Status;
    Status = CFE_SRL_HandleClose(CFE_SRL_ApiGetHandle(Cmd->Payload.Indexer));
    
    if(Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(CFE_SRL_CLOSE_HANDLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Close Handle Cmd Success.");
    }
    else {
        CFE_StatusString_t String;

        CFE_ES_StatusToString(Status, &String);
        CFE_EVS_SendEvent(CFE_SRL_CLOSE_HANDLE_INF_EID, CFE_EVS_EventType_ERROR, "SRL Close Handle failed. RC=%s", String);
    }

    return CFE_SUCCESS;
}

int32 CFE_SRL_SendHkCmd(const CFE_SRL_SendHkCmd_t *data) {
    /**
     * Get IO Handle Status
     */
    for (uint8_t i=0; i < CFE_SRL_GNRL_DEVICE_NUM; i++) {
        CFE_SRL_IO_Handle_t *TempHandle = CFE_SRL_ApiGetHandle(i);

        CFE_SRL_TaskData.HKTlmMsg.Payload.IOHandleStatus[i] = 
        ((const CFE_SRL_Global_Handle_t *)TempHandle)->Status;

        CFE_SRL_TaskData.HKTlmMsg.Payload.IOHandle[i] = *TempHandle;
    }

    /**
     * Get GPIO Handle Status
     */
    for (uint8_t i=0; i < CFE_SRL_TOT_GPIO_NUM; i++) {
        CFE_SRL_TaskData.HKTlmMsg.Payload.GPIOHandle[i] = *CFE_SRL_ApiGetGpioHandle(i);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CFE_SRL_TaskData.HKTlmMsg.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(CFE_SRL_TaskData.HKTlmMsg.TelemetryHeader), true);

    CFE_EVS_SendEvent(114, CFE_EVS_EventType_INFORMATION, "SRL Send HK Cmd Received.");

    return CFE_SUCCESS;
}