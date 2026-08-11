/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_dispatch.h"
#include "pay_slt_eventids.h"
#include "pay_slt_version.h"

PAY_SLT_Data_t PAY_SLT_Data;

void PAY_SLT_Main(void)
{
    CFE_Status_t status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(PAY_SLT_PERF_ID);

    status = PAY_SLT_Init();
    if (status != CFE_SUCCESS)
    {
        PAY_SLT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&PAY_SLT_Data.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(PAY_SLT_PERF_ID);
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAY_SLT_Data.CommandPipe, CFE_SB_PEND_FOREVER);
        CFE_ES_PerfLogEntry(PAY_SLT_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            PAY_SLT_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PAY_SLT_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT: SB Pipe Read Error, App Will Exit");
            PAY_SLT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(PAY_SLT_PERF_ID);
    CFE_ES_ExitApp(PAY_SLT_Data.RunStatus);
}

CFE_Status_t PAY_SLT_Init(void)
{
    CFE_Status_t status;

    memset(&PAY_SLT_Data, 0, sizeof(PAY_SLT_Data));
    PAY_SLT_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    PAY_SLT_Data.PipeDepth = PAY_SLT_PIPE_DEPTH;
    PAY_SLT_Data.BcnEnabled = PAY_SLT_BCN_ENABLED;
    PAY_SLT_Data.HkEnabled = PAY_SLT_HK_ENABLED;

    strncpy(PAY_SLT_Data.PipeName, "PAY_SLT_CMD_PIPE", sizeof(PAY_SLT_Data.PipeName));
    PAY_SLT_Data.PipeName[sizeof(PAY_SLT_Data.PipeName) - 1] = 0;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAY_SLT: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    status = CFE_MSG_Init(CFE_MSG_PTR(PAY_SLT_Data.HkTlm.TelemetryHeader),
                          CFE_SB_ValueToMsgId(PAY_SLT_HK_TLM_MID), sizeof(PAY_SLT_Data.HkTlm));
    if (status == CFE_SUCCESS)
    {
        status = CFE_MSG_Init(CFE_MSG_PTR(PAY_SLT_Data.BcnTlm.TelemetryHeader),
                              CFE_SB_ValueToMsgId(PAY_SLT_BCN_TLM_MID), sizeof(PAY_SLT_Data.BcnTlm));
    }
    if (status == CFE_SUCCESS)
    {
        status = CFE_MSG_Init(CFE_MSG_PTR(PAY_SLT_Data.RptPkt.TelemetryHeader),
                              CFE_SB_ValueToMsgId(PAY_SLT_RPT_TLM_MID), sizeof(PAY_SLT_Data.RptPkt));
    }
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAY_SLT_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: telemetry message initialization failed, RC = 0x%08lX",
                          (unsigned long)status);
        return status;
    }

    status = CFE_SB_CreatePipe(&PAY_SLT_Data.CommandPipe, PAY_SLT_Data.PipeDepth, PAY_SLT_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAY_SLT_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAY_SLT_SEND_HK_MID), PAY_SLT_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAY_SLT_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAY_SLT_SEND_BCN_MID), PAY_SLT_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAY_SLT_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAY_SLT_CMD_MID), PAY_SLT_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAY_SLT_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Error Subscribing to CMD MID, RC = 0x%08lX", (unsigned long)status);
        return status;
    }


    PAY_SLT_Data.RS422Handle = CFE_SRL_ApiGetHandle(CFE_SRL_RS422_HANDLE_INDEXER);
    
    if (PAY_SLT_Data.RS422Handle == NULL) {
        CFE_EVS_SendEvent(PAY_SLT_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Failed to get RS422 handle");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    PAY_SLT_Data.I2c1Handle = CFE_SRL_ApiGetHandle(CFE_SRL_I2C1_HANDLE_INDEXER);
    if (PAY_SLT_Data.I2c1Handle == NULL) {
        CFE_EVS_SendEvent(PAY_SLT_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Failed to get I2C1 handle");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_EVS_SendEvent(PAY_SLT_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "PAY_SLT Initialized");

    return CFE_SUCCESS;
}
