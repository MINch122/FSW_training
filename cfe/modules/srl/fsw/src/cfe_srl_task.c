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

#include "cfe_version.h"
#include "cfe_config.h"
#include "target_config.h"

#include <string.h>

/**
 * Serial Service (SRL) Task global data;
 */
CFE_SRL_Global_t CFE_SRL_Global;

/* Serial Handle for each srl comm. */
extern CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];

static bool CFE_SRL_IsValidIndexer(CFE_SRL_Handle_Indexer_t Indexer)
{
    return (int32)Indexer >= 0 && (uint32)Indexer < CFE_SRL_GNRL_DEVICE_NUM;
}

static int32 CFE_SRL_FillHandleReport(CFE_SRL_Handle_Indexer_t Indexer,
                                      CFE_SRL_HandleStatusReport_t *Report,
                                      bool UpdateCounters)
{
    CFE_SRL_IO_Handle_t *Handle;
    const CFE_SRL_Global_Handle_t *Entry;
    int32 Status = CFE_SUCCESS;

    if (Report == NULL)
    {
        return CFE_SRL_BAD_ARGUMENT;
    }

    memset(Report, 0, sizeof(*Report));
    Report->Indexer = (uint8)Indexer;
    Report->FD = -1;

    if (!CFE_SRL_IsValidIndexer(Indexer))
    {
        return CFE_STATUS_RANGE_ERROR;
    }

    Handle = Handles[(uint32)Indexer];
    if (Handle == NULL)
    {
        return CFE_SRL_BAD_ARGUMENT;
    }

    Entry = (const CFE_SRL_Global_Handle_t *)Handle;
    Report->Status = Entry->Status;
    Report->DevType = (uint8)Entry->DevType;
    Report->FD = Handle->FD;
    strncpy(Report->Name, Entry->Name, sizeof(Report->Name) - 1);
    strncpy(Report->DevName, Entry->DevName, sizeof(Report->DevName) - 1);

    if (UpdateCounters)
    {
        Status = CFE_SRL_UpdateHandleCounters(Handle);
    }
    Report->Counters = Handle->Counters;

    return Status;
}

CFE_Status_t CFE_SRL_SendReport(const void *Cmd, uint8 ReturnType, int32 ReturnCode,
                                const void *Data, size_t DataSize)
{
    CFE_SB_Buffer_t *BufPtr;
    CFE_SRL_ReportTlm_t *Report;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_Status_t Status;
    uint16 CopySize = 0;

    if (Cmd == NULL)
    {
        return CFE_SRL_BAD_ARGUMENT;
    }

    CFE_MSG_GetMsgId((const CFE_MSG_Message_t *)Cmd, &MsgId);
    CFE_MSG_GetFcnCode((const CFE_MSG_Message_t *)Cmd, &CommandCode);

    BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(CFE_SRL_ReportTlm_t));
    if (BufPtr == NULL)
    {
        CFE_EVS_SendEvent(CFE_SRL_REPORT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SRL report allocation failed CC=%u", (unsigned)CommandCode);
        return CFE_SB_BUF_ALOC_ERR;
    }

    Report = (CFE_SRL_ReportTlm_t *)BufPtr;
    memset(Report, 0, sizeof(*Report));
    Status = CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader),
                          CFE_SB_ValueToMsgId(CFE_SRL_REPORT_TLM_MID), sizeof(*Report));
    if (Status != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        CFE_EVS_SendEvent(CFE_SRL_REPORT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SRL report init failed CC=%u RC=0x%08lX",
                          (unsigned)CommandCode, (unsigned long)Status);
        return Status;
    }

    if (Data != NULL && DataSize > 0)
    {
        CopySize = (DataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : (uint16)DataSize;
        memcpy(Report->Report.ReturnValue, Data, CopySize);
    }

    Report->Report.MsgID = (uint16)CFE_SB_MsgIdToValue(MsgId);
    Report->Report.CommandCode = (uint8)CommandCode;
    Report->Report.ReturnType = ReturnType;
    Report->Report.ReturnCode = ReturnCode;
    Report->Report.ReturnDataSize = CopySize;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer(BufPtr, true);
    if (Status != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        CFE_EVS_SendEvent(CFE_SRL_REPORT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SRL report transmit failed CC=%u RC=0x%08lX",
                          (unsigned)CommandCode, (unsigned long)Status);
    }

    return Status;
}

static void CFE_SRL_ReportCommandResult(const void *Cmd, int32 Status,
                                        const void *Data, size_t DataSize)
{
    uint8 ReturnType = RPT_RETTYPE_SUCCESS;

    if (Status != CFE_SUCCESS)
    {
        ReturnType = RPT_RETTYPE_CFE;
        CFE_SRL_Global.HKTlmMsg.Payload.CommandErrorCounter++;
    }

    CFE_SRL_SendReport(Cmd, ReturnType, Status, Data, DataSize);
}

/* GPIO Handle for each gpio */
// extern CFE_SRL_GPIO_Handle_t GPIO[CFE_SRL_TOT_GPIO_NUM];

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
        // OS_printf("SRL TaskMain Loop %u\n", (unsigned int)sizeof(CFE_SRL_IO_Handle_t));
        /*
        ** Performance Time Stamp Exit
        */
        CFE_ES_PerfLogExit(CFE_MISSION_SRL_MAIN_PERF_ID);
        
        /**
        * Wait for the next Software Bus message.
        */
        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, CFE_SRL_Global.CmdPipe, CFE_SB_PEND_FOREVER);

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
    CFE_ES_GetAppID(&CFE_SRL_Global.AppId);

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
    CFE_MSG_Init(CFE_MSG_PTR(CFE_SRL_Global.HKTlmMsg.TelemetryHeader), CFE_SB_ValueToMsgId(CFE_SRL_HK_TLM_MID),
                sizeof(CFE_SRL_Global.HKTlmMsg));

    Status = CFE_SB_CreatePipe(&CFE_SRL_Global.CmdPipe, CFE_SRL_PIPE_DEPTH, CFE_SRL_PIPE_NAME);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Call to CFE_SB_CreatePipe Failed. RC=0x%08X\n", __func__, Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CFE_SRL_CMD_MID), CFE_SRL_Global.CmdPipe);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Subscribe to Cmds Failed. RC=0x%08X\n", __func__, Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CFE_SRL_SEND_HK_MID), CFE_SRL_Global.CmdPipe);
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
    
    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter++;

    CFE_EVS_SendEvent(CFE_SRL_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                        "SRL No-op command: %s", VersionString);

    CFE_SRL_ReportCommandResult(Cmd, CFE_SUCCESS, VersionString, strlen(VersionString) + 1);
    
    return CFE_SUCCESS;
}

int32 CFE_SRL_ResetCounterCmd(const CFE_SRL_ResetCounterCmd_t *Cmd) {

    uint8 Counters[2];

    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter = 0;
    CFE_SRL_Global.HKTlmMsg.Payload.CommandErrorCounter = 0;

    Counters[0] = CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter;
    Counters[1] = CFE_SRL_Global.HKTlmMsg.Payload.CommandErrorCounter;

    CFE_EVS_SendEvent(CFE_SRL_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Reset Counter Cmd Received.");

    CFE_SRL_ReportCommandResult(Cmd, CFE_SUCCESS, Counters, sizeof(Counters));

    return CFE_SUCCESS;
}

int32 CFE_SRL_ResetHandleCounterCmd(const CFE_SRL_ResetHandleCounterCmd_t *Cmd) {
    CFE_SRL_IO_Handle_t *TempHandle = NULL;
    CFE_PSP_IODriver_Location_t Location = {0};
    CFE_SRL_HandleStatusReport_t Report;
    int32 Status;

    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter++;

    if (!CFE_SRL_IsValidIndexer(Cmd->Payload.Indexer))
    {
        Status = CFE_STATUS_RANGE_ERROR;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));
        return CFE_SUCCESS;
    }

    TempHandle = Handles[(uint32)Cmd->Payload.Indexer];
    if (TempHandle == NULL)
    {
        Status = CFE_SRL_BAD_ARGUMENT;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));
        return CFE_SUCCESS;
    }

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_CONFIG_SUBSYSTEM;
    Location.SubchannelId = 0; // Meaningless

    Status = CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_SERIAL_IO_CLEAR_CNTS,
                                      CFE_PSP_IODriver_U32ARG(TempHandle->FD));

    if (Status == CFE_SUCCESS)
    {
        Status = CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, true);
    }
    else
    {
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
    }

    CFE_EVS_SendEvent(CFE_SRL_RESET_HANDLE_INF_EID,
                      (Status == CFE_SUCCESS) ? CFE_EVS_EventType_INFORMATION : CFE_EVS_EventType_ERROR,
                      "SRL Reset Handle Counter Cmd RC=0x%08lX", (unsigned long)Status);
    CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));

    return CFE_SUCCESS;
    
}

/**
 * @deprecated Use `CFE_SRL_SendHkCmd` instead
 */
int32 CFE_SRL_GetHandleStatusCmd(const CFE_SRL_GetHandleStatusCmd_t *Cmd) {

    CFE_SRL_HandleStatusReport_t Report;
    int32 Status;

    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter++;
    Status = CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, true);

    CFE_EVS_SendEvent(CFE_SRL_GET_HANDLE_STATUS_INF_EID,
                      (Status == CFE_SUCCESS) ? CFE_EVS_EventType_INFORMATION : CFE_EVS_EventType_ERROR,
                      "SRL Get Handle Status index=%ld RC=0x%08lX",
                      (long)Cmd->Payload.Indexer, (unsigned long)Status);
    CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));

    return CFE_SUCCESS;
}

int32 CFE_SRL_InitHandleCmd(const CFE_SRL_InitHandleCmd_t *Cmd) {
    int32 Status;
    CFE_SRL_HandleStatusReport_t Report;

    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter++;

    if (!CFE_SRL_IsValidIndexer(Cmd->Payload.Indexer))
    {
        Status = CFE_STATUS_RANGE_ERROR;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));
        return CFE_SUCCESS;
    }

    if (memchr(Cmd->Payload.Name, '\0', sizeof(Cmd->Payload.Name)) == NULL ||
        memchr(Cmd->Payload.DevName, '\0', sizeof(Cmd->Payload.DevName)) == NULL)
    {
        Status = CFE_STATUS_VALIDATION_FAILURE;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));
        return CFE_SUCCESS;
    }

    /**
     * In this command, data interface configuration is setted to defualt
     * Specification of data interface is shown at below as source code
     */
    CFE_SRL_IO_Config_t Config = {0, }; /* FD will be setted automatically */
    switch (Cmd->Payload.DevType)
    {
    case SRL_DEVTYPE_I2C:
        Config.cfg.i2c = (CFE_PSP_I2C_cfg_t) {.pec_en = false,
                                              .retries = 3,
                                              .tenbit = false};
        break;
    case SRL_DEVTYPE_SPI:
        Config.cfg.spi = (CFE_PSP_SPI_cfg_t) {.bpw = 8,
                                              .mode = 0,
                                              .speed = 2000000};
        break;
    case SRL_DEVTYPE_CAN:
        /**
         * Do nothing
         * Struct is already cleared to All-zero
         * Off various settings (e.g. filter, loopback, etc...)
         */
        break;
    case SRL_DEVTYPE_UART:
    case SRL_DEVTYPE_RS422:
        Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 115200,
                                                .databits = 8,
                                                .parity = 0,
                                                .stopbits = 1};
        break;

    default:
        Status = CFE_STATUS_RANGE_ERROR;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));
        return CFE_SUCCESS;
    }
    
    Status = CFE_SRL_HandleInit(&Handles[(uint32)Cmd->Payload.Indexer], Cmd->Payload.Name,
                                Cmd->Payload.DevName, Cmd->Payload.DevType,
                                Cmd->Payload.Indexer, &Config);
    if (Status == CFE_SUCCESS)
        CFE_EVS_SendEvent(CFE_SRL_INIT_HANDLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Init Handle Cmd Success.");
    else
        CFE_EVS_SendEvent(CFE_SRL_INIT_HANDLE_INF_EID, CFE_EVS_EventType_ERROR, "SRL Init Handle Cmd failed. RC=0x%08X", Status);
    if (Status == CFE_SUCCESS)
    {
        Status = CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
    }
    else
    {
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
    }
    CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));

    return CFE_SUCCESS;
}

int32 CFE_SRL_CloseHandleCmd(const CFE_SRL_CloseHandleCmd_t *Cmd) {
    int32 Status;
    CFE_SRL_HandleStatusReport_t Report;

    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter++;

    Status = CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
    if (Status == CFE_SUCCESS)
    {
        Status = CFE_SRL_HandleClose(&Handles[(uint32)Cmd->Payload.Indexer]);
    }

    if(Status == CFE_SUCCESS) {
        Report.Status = CFE_SRL_HANDLE_STATUS_NONE;
        Report.FD = -1;
        CFE_EVS_SendEvent(CFE_SRL_CLOSE_HANDLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Close Handle Cmd Success.");
    }
    else {
        CFE_EVS_SendEvent(CFE_SRL_CLOSE_HANDLE_INF_EID, CFE_EVS_EventType_ERROR, "SRL Close Handle failed. RC=0x%08X", Status);
    }

    CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));

    return CFE_SUCCESS;
}

int32 CFE_SRL_ConfigHandleCmd(const CFE_SRL_ConfigHandleCmd_t *Cmd) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_SRL_IO_Handle_t *Handle;
    CFE_SRL_HandleStatusReport_t Report;
    bool IsLocked = false;

    CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter++;

    if (!CFE_SRL_IsValidIndexer(Cmd->Payload.Indexer))
    {
        Status = CFE_STATUS_RANGE_ERROR;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        goto report;
    }

    /* Validataion */
    Handle = Handles[(uint32)Cmd->Payload.Indexer];
    if (Handle == NULL) {
        CFE_EVS_SendErr(CFE_SRL_CONFIG_HANDLE_INF_EID, "SRL Config failed. Not initialized FD.");
        Status = CFE_SRL_BAD_ARGUMENT;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        goto report;
    }

    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(CFE_SRL_CONFIG_HANDLE_INF_EID, CFE_EVS_EventType_ERROR, "SRL Config Handle lock failed. RC=0x%08X", Status);
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        goto report;
    }
    IsLocked = true;

    if (Handle->FD != Cmd->Payload.Config.FD) {
        CFE_EVS_SendErr(CFE_SRL_CONFIG_HANDLE_INF_EID, "SRL Config failed. Wrong FD.");
        Status = CFE_SRL_BAD_ARGUMENT;
        CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);
        goto report;
    }

    /* Config function */
    DevType = CFE_SRL_GetHandleDevType(Handle);
    Status = CFE_SRL_ConfigHandle(DevType, (CFE_PSP_IODriver_Serial_cfg_t *)&Cmd->Payload.Config);
    CFE_SRL_FillHandleReport(Cmd->Payload.Indexer, &Report, false);

    if(Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(CFE_SRL_CONFIG_HANDLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SRL Config Handle Cmd Success.");
    }
    else {
        CFE_EVS_SendEvent(CFE_SRL_CONFIG_HANDLE_INF_EID, CFE_EVS_EventType_ERROR, "SRL Config Handle failed. RC=0x%08X", Status);
    }

report:
    if (IsLocked)
    {
        int32 UnlockStatus = CFE_SRL_MutexUnlock(Handle);
        if (Status == CFE_SUCCESS && UnlockStatus != CFE_SUCCESS)
        {
            Status = UnlockStatus;
        }
    }
    CFE_SRL_ReportCommandResult(Cmd, Status, &Report, sizeof(Report));

    return CFE_SUCCESS;
}

int32 CFE_SRL_SendHkCmd(const CFE_SRL_SendHkCmd_t *data) {
    int32 Status = CFE_SUCCESS;
    /**
     * Get IO Handle Status
     */
    for (uint8_t i = 0; i < CFE_SRL_GNRL_DEVICE_NUM; i++) {
        CFE_SRL_IO_Handle_t *TempHandle = CFE_SRL_ApiGetHandle(i);
        if (TempHandle == NULL) { // If handle closed, put 0 to Tlm
            CFE_SRL_Global.HKTlmMsg.Payload.IOHandleStatus[i] = 0;
            CFE_SRL_Global.HKTlmMsg.Payload.IOHandleTxCount[i] = 0;
            continue;
        }

        CFE_SRL_Global.HKTlmMsg.Payload.IOHandleStatus[i] = 
        ((const CFE_SRL_Global_Handle_t *)TempHandle)->Status;

        int32 UpdateStatus = CFE_SRL_UpdateHandleCounters(TempHandle);
        if (UpdateStatus == CFE_SUCCESS) {
            CFE_SRL_Global.HKTlmMsg.Payload.IOHandleTxCount[i] = TempHandle->Counters.TxCnt;
            OS_printf("%s: %s Tx Cnt : %u || Rx Cnt : %u || Tx Err : %u || Rx Err : %u\n", __func__, ((const CFE_SRL_Global_Handle_t *)TempHandle)->Name, TempHandle->Counters.TxCnt, TempHandle->Counters.RxCnt, TempHandle->Counters.TxErr, TempHandle->Counters.RxErr);
        }
        else if (Status == CFE_SUCCESS)
        {
            Status = UpdateStatus;
        }
    }

    /**
     * Get GPIO Handle Status
     */
    // for (uint8_t i=0; i < CFE_SRL_TOT_GPIO_NUM; i++) {
    //     CFE_SRL_Global.HKTlmMsg.Payload.GPIOHandle[i] = *CFE_SRL_ApiGetGpioHandle(i);
    // }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CFE_SRL_Global.HKTlmMsg.TelemetryHeader));
    int32 TransmitStatus = CFE_SB_TransmitMsg(CFE_MSG_PTR(CFE_SRL_Global.HKTlmMsg.TelemetryHeader), true);
    if (Status == CFE_SUCCESS)
    {
        Status = TransmitStatus;
    }

    CFE_SRL_ReportCommandResult(data, Status, &CFE_SRL_Global.HKTlmMsg.Payload,
                                sizeof(CFE_SRL_Global.HKTlmMsg.Payload));
    
    // CFE_EVS_SendEvent(114, CFE_EVS_EventType_INFORMATION, "SRL Send HK Cmd Received.");

    return CFE_SUCCESS;
}
