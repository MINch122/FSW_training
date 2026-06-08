#include "ftp_client_app.h"
#include "ftp_client_dispatch.h"
#include "ftp_client_eventids.h"

#include <csp/csp.h>
#include <csp/csp_buffer.h>
#include <stdio.h>
#include <string.h>

FTP_CLIENT_AppData_t FTP_CLIENT_AppData;

#define FTP_CLIENT_CSP_BUF_MON_TASK_NAME       "FTP_CSP_BUF_MON"
#define FTP_CLIENT_CSP_BUF_MON_STACK_SIZE      (4 * 1024)
#define FTP_CLIENT_CSP_BUF_MON_PRIORITY        200
#define FTP_CLIENT_CSP_BUF_MON_PATH            "/cf/cspbuf.txt"
#define FTP_CLIENT_CSP_BUF_MON_SAMPLE_MS       100
#define FTP_CLIENT_CSP_BUF_MON_PRINT_MS        10000

static osal_id_t FTP_CLIENT_CspBufferMonitorTaskId;

static void FTP_CLIENT_CspBufferMonitorTask(void)
{
    osal_id_t FileHandle;
    int32     Status;
    uint32    Sample = 0;
    uint32    TimeMs = 0;
    uint32    PrintDivider;
    char      Line[96];
    int       LineLen;

    for (int i = 0; i < 10; i++) {
        OS_printf("FTP_CLIENT CSP buffer monitor: waiting for CSP initialization... (%d/10)\n", i + 1);
        OS_TaskDelay(1000);
    }

    Status = OS_OpenCreate(&FileHandle, FTP_CLIENT_CSP_BUF_MON_PATH,
                           OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_WRITE_ONLY);
    if (Status != OS_SUCCESS) {
        OS_printf("FTP_CLIENT CSP buffer monitor: failed to open %s, RC=%d\n",
                  FTP_CLIENT_CSP_BUF_MON_PATH, (int)Status);
        return;
    }

    (void)OS_write(FileHandle, "sample,time_ms,remaining,total,used\n",
                   strlen("sample,time_ms,remaining,total,used\n"));

    PrintDivider = FTP_CLIENT_CSP_BUF_MON_PRINT_MS / FTP_CLIENT_CSP_BUF_MON_SAMPLE_MS;
    if (PrintDivider == 0) {
        PrintDivider = 1;
    }

    while (true) {
        const csp_conf_t *Conf = csp_get_conf();
        uint32           Total = Conf->buffers;
        uint32           Remaining = (uint32)csp_buffer_remaining();
        uint32           Used = (Remaining <= Total) ? (Total - Remaining) : 0;

        LineLen = snprintf(Line, sizeof(Line), "%u,%u,%u,%u,%u\n",
                           (unsigned int)Sample,
                           (unsigned int)TimeMs,
                           (unsigned int)Remaining,
                           (unsigned int)Total,
                           (unsigned int)Used);
        if (LineLen > 0 && Used > 0) {
            size_t WriteLen = (LineLen < (int)sizeof(Line)) ? (size_t)LineLen : (sizeof(Line) - 1);
            (void)OS_write(FileHandle, Line, WriteLen);
        }

        if ((Sample % PrintDivider) == 0) {
            OS_printf("FTP_CLIENT CSP buffer: remaining=%u total=%u used=%u\n",
                      (unsigned int)Remaining,
                      (unsigned int)Total,
                      (unsigned int)Used);
        }

        Sample++;
        TimeMs += FTP_CLIENT_CSP_BUF_MON_SAMPLE_MS;
        OS_TaskDelay(FTP_CLIENT_CSP_BUF_MON_SAMPLE_MS);
    }
}

static int32 FTP_CLIENT_StartCspBufferMonitor(void)
{
    int32 Status;

    Status = OS_TaskCreate(&FTP_CLIENT_CspBufferMonitorTaskId,
                           FTP_CLIENT_CSP_BUF_MON_TASK_NAME,
                           FTP_CLIENT_CspBufferMonitorTask,
                           OSAL_TASK_STACK_ALLOCATE,
                           FTP_CLIENT_CSP_BUF_MON_STACK_SIZE,
                           FTP_CLIENT_CSP_BUF_MON_PRIORITY,
                           0);
    if (Status != OS_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: CSP buffer monitor task create failed. RC=%d\n",
                             __func__, (int)Status);
    } else {
        CFE_ES_WriteToSysLog("%s: CSP buffer monitor task created successfully.\n", __func__);
    }

    return Status;
}

void FTP_CLIENT_Main(void)
{
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(FTP_CLIENT_PERF_ID);

    Status = FTP_CLIENT_Init();
    if (Status != CFE_SUCCESS) {
        FTP_CLIENT_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&FTP_CLIENT_AppData.RunStatus) == true) {
        CFE_ES_PerfLogExit(FTP_CLIENT_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, FTP_CLIENT_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(FTP_CLIENT_PERF_ID);

        if (Status == CFE_SUCCESS) {
            FTP_CLIENT_TaskPipe(SBBufPtr);
        } else {
            CFE_EVS_SendEvent(FTP_CLIENT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "FTP_CLIENT: SB pipe read error, app will exit");
            FTP_CLIENT_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(FTP_CLIENT_PERF_ID);
    CFE_ES_ExitApp(FTP_CLIENT_AppData.RunStatus);
}

CFE_Status_t FTP_CLIENT_Init(void)
{
    CFE_Status_t Status;

    memset(&FTP_CLIENT_AppData, 0, sizeof(FTP_CLIENT_AppData));
    FTP_CLIENT_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    FTP_CLIENT_AppData.PipeDepth = FTP_CLIENT_PIPE_DEPTH;

    strncpy(FTP_CLIENT_AppData.PipeName, "FTP_CLIENT_CMD", sizeof(FTP_CLIENT_AppData.PipeName));
    FTP_CLIENT_AppData.PipeName[sizeof(FTP_CLIENT_AppData.PipeName) - 1] = 0;

    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("FTP_CLIENT: Error registering events, RC = 0x%08lX\n",
                             (unsigned long)Status);
        return Status;
    }

    Status = CFE_SB_CreatePipe(&FTP_CLIENT_AppData.CommandPipe,
                               FTP_CLIENT_AppData.PipeDepth,
                               FTP_CLIENT_AppData.PipeName);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(FTP_CLIENT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "FTP_CLIENT: Error creating command pipe, RC = 0x%08lX",
                          (unsigned long)Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(FTP_CLIENT_CMD_MID),
                              FTP_CLIENT_AppData.CommandPipe);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(FTP_CLIENT_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "FTP_CLIENT: Error subscribing to commands, RC = 0x%08lX",
                          (unsigned long)Status);
        return Status;
    }

    Status = FTP_CLIENT_StartCspBufferMonitor();
    if (Status != OS_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: CSP buffer monitor start failed! RC=%d\n", __func__, (int)Status);
    }

    CFE_EVS_SendEvent(FTP_CLIENT_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "FTP_CLIENT initialized");

    return CFE_SUCCESS;
}

void FTP_CLIENT_SendReport(const CFE_MSG_Message_t *CmdMsg,
                           const void *Data,
                           uint16 DataSize,
                           int32 ReturnCode,
                           uint8 ReturnType)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;
    uint16 CopySize = DataSize;

    if (CopySize > RPT_RET_VALUE_BUF_SIZE) {
        CopySize = RPT_RET_VALUE_BUF_SIZE;
    }

    memset(&FTP_CLIENT_AppData.ReportTlm, 0, sizeof(FTP_CLIENT_AppData.ReportTlm));
    CFE_MSG_Init(CFE_MSG_PTR(FTP_CLIENT_AppData.ReportTlm.TelemetryHeader),
                 CFE_SB_ValueToMsgId(FTP_CLIENT_REPORT_TLM_MID),
                 sizeof(FTP_CLIENT_AppData.ReportTlm));

    if (CmdMsg != NULL) {
        CFE_MSG_GetMsgId(CmdMsg, &MsgId);
        CFE_MSG_GetFcnCode(CmdMsg, &CommandCode);
    }

    FTP_CLIENT_AppData.ReportTlm.Payload.MsgID = (uint16)CFE_SB_MsgIdToValue(MsgId);
    FTP_CLIENT_AppData.ReportTlm.Payload.CommandCode = CommandCode;
    FTP_CLIENT_AppData.ReportTlm.Payload.ReturnType = ReturnType;
    FTP_CLIENT_AppData.ReportTlm.Payload.ReturnCode = ReturnCode;
    FTP_CLIENT_AppData.ReportTlm.Payload.ReturnDataSize = CopySize;

    if (Data != NULL && CopySize > 0) {
        memcpy(FTP_CLIENT_AppData.ReportTlm.Payload.ReturnValue, Data, CopySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(FTP_CLIENT_AppData.ReportTlm.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(FTP_CLIENT_AppData.ReportTlm.TelemetryHeader), true);
}
