#include "meow.h"
#include "meow_cmds.h"
#include "meow_dispatch.h"
#include "meow_eventids.h"
#include "meow_version.h"

MEOW_AppData_t MEOW_AppData;

void MEOW_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(MEOW_PERF_ID);

    status = MEOW_Init();
    if (status != CFE_SUCCESS)
        MEOW_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;

    while (CFE_ES_RunLoop(&MEOW_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(MEOW_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, MEOW_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(MEOW_PERF_ID);

        if (status == CFE_SUCCESS)
            MEOW_TaskPipe(SBBufPtr);
        else
        {
            CFE_EVS_SendEvent(MEOW_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MEOW: SB Pipe Read Error, App Will Exit");
            MEOW_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(MEOW_PERF_ID);
    CFE_ES_ExitApp(MEOW_AppData.RunStatus);
}

CFE_Status_t MEOW_Init(void)
{
    CFE_Status_t status;
    char         VersionString[MEOW_CFG_MAX_VERSION_STR_LEN];

    memset(&MEOW_AppData, 0, sizeof(MEOW_AppData));
    MEOW_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    MEOW_AppData.PipeDepth = MEOW_PLATFORM_PIPE_DEPTH;
    strncpy(MEOW_AppData.PipeName, MEOW_PLATFORM_PIPE_NAME, sizeof(MEOW_AppData.PipeName));
    MEOW_AppData.PipeName[sizeof(MEOW_AppData.PipeName) - 1] = 0;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MEOW: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    CFE_MSG_Init(CFE_MSG_PTR(MEOW_AppData.HkTlm.TelemetryHeader),
                 CFE_SB_ValueToMsgId(MEOW_HK_TLM_MID),
                 sizeof(MEOW_AppData.HkTlm));

    CFE_MSG_Init(CFE_MSG_PTR(MEOW_AppData.Report.TelemetryHeader),
                 CFE_SB_ValueToMsgId(MEOW_REPORT_TLM_MID),
                 sizeof(MEOW_AppData.Report));

    status = CFE_SB_CreatePipe(&MEOW_AppData.CommandPipe, MEOW_AppData.PipeDepth, MEOW_AppData.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MEOW_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MEOW_SEND_HK_MID), MEOW_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MEOW_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MEOW_CMD_MID), MEOW_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MEOW_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    CFE_Config_GetVersionString(VersionString, MEOW_CFG_MAX_VERSION_STR_LEN, "Meow",
                                MEOW_VERSION, MEOW_BUILD_CODENAME, MEOW_LAST_OFFICIAL);
    CFE_EVS_SendEvent(MEOW_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "MEOW Initialized.%s", VersionString);

    return CFE_SUCCESS;
}
