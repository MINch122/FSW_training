/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_dispatch.h"
#include "pay_slt_eventids.h"
#include "pay_slt_version.h"

SLT_IFB_Data_t SLT_IFB_Data;

void PAY_SLT_Main(void)
{
    CFE_Status_t status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(SLT_IFB_PERF_ID);

    status = SLT_IFB_Init();
    if (status != CFE_SUCCESS)
    {
        SLT_IFB_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&SLT_IFB_Data.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(SLT_IFB_PERF_ID);
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, SLT_IFB_Data.CommandPipe, CFE_SB_PEND_FOREVER);
        CFE_ES_PerfLogEntry(SLT_IFB_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SLT_IFB_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(SLT_IFB_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT: SB Pipe Read Error, App Will Exit");
            SLT_IFB_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(SLT_IFB_PERF_ID);
    CFE_ES_ExitApp(SLT_IFB_Data.RunStatus);
}

CFE_Status_t SLT_IFB_Init(void)
{
    CFE_Status_t status;
    char VersionString[SLT_IFB_CFG_MAX_VERSION_STR_LEN];

    memset(&SLT_IFB_Data, 0, sizeof(SLT_IFB_Data));
    SLT_IFB_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    SLT_IFB_Data.PipeDepth = SLT_IFB_PIPE_DEPTH;
    SLT_IFB_Data.BcnSbEnabled = PAY_SLT_SB_BCN_ENABLED;

    strncpy(SLT_IFB_Data.PipeName, "PAY_SLT_CMD_PIPE", sizeof(SLT_IFB_Data.PipeName));
    SLT_IFB_Data.PipeName[sizeof(SLT_IFB_Data.PipeName) - 1] = 0;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAY_SLT: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    CFE_MSG_Init(CFE_MSG_PTR(SLT_IFB_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAY_SLT_HK_TLM_MID),
                 sizeof(SLT_IFB_Data.HkTlm));
    CFE_MSG_Init(CFE_MSG_PTR(SLT_IFB_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAY_SLT_BCN_TLM_MID),
                 sizeof(SLT_IFB_Data.BcnTlm));
    CFE_MSG_Init(CFE_MSG_PTR(SLT_IFB_Data.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(PAY_SLT_RPT_TLM_MID),
                 sizeof(SLT_IFB_Data.RptPkt));

    status = CFE_SB_CreatePipe(&SLT_IFB_Data.CommandPipe, SLT_IFB_Data.PipeDepth, SLT_IFB_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SLT_IFB_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAY_SLT_SEND_HK_MID), SLT_IFB_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SLT_IFB_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAY_SLT_SEND_BCN_MID), SLT_IFB_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SLT_IFB_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAY_SLT_CMD_MID), SLT_IFB_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SLT_IFB_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error Subscribing to CMD MID, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    
#ifdef PAY_SLT_I2C_HANDLE_INDEXER
    SLT_IFB_Data.I2c1Handle = CFE_SRL_ApiGetHandle(PAY_SLT_I2C_HANDLE_INDEXER);
    if (SLT_IFB_Data.I2c1Handle == NULL)
    {
        CFE_EVS_SendEvent(SLT_IFB_INIT_INF_EID, CFE_EVS_EventType_ERROR, "PAY_SLT: Failed to get I2C1 handle");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
#else
    SLT_IFB_Data.I2c1Handle = NULL;
    CFE_EVS_SendEvent(SLT_IFB_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAY_SLT: I2C1 handle is not configured; I2C download is disabled");
#endif

    CFE_Config_GetVersionString(VersionString, SLT_IFB_CFG_MAX_VERSION_STR_LEN, "PAY_SLT App", SLT_IFB_VERSION,
                                SLT_IFB_BUILD_CODENAME, SLT_IFB_LAST_OFFICIAL);
    CFE_EVS_SendEvent(SLT_IFB_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "PAY_SLT Initialized.%s", VersionString);

    return CFE_SUCCESS;
}
