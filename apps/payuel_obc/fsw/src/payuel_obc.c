#include "payuel_obc.h"
#include "payuel_obc_cmds.h"
#include "payuel_obc_child.h"
#include "payuel_obc_eventids.h"
#include "payuel_obc_dispatch.h"
#include "payuel_obc_version.h"

PAYUEL_OBC_Data_t PAYUEL_OBC_Data;

void PAYUEL_OBC_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(PAYUEL_OBC_PERF_ID);

    status = PAYUEL_OBC_Init();
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&PAYUEL_OBC_Data.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(PAYUEL_OBC_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYUEL_OBC_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(PAYUEL_OBC_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            PAYUEL_OBC_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: SB Pipe Read Error, App Will Exit");
            PAYUEL_OBC_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(PAYUEL_OBC_PERF_ID);
    CFE_ES_ExitApp(PAYUEL_OBC_Data.RunStatus);
}

CFE_Status_t PAYUEL_OBC_Init(void)
{
    CFE_Status_t status;

    memset(&PAYUEL_OBC_Data, 0, sizeof(PAYUEL_OBC_Data));
    PAYUEL_OBC_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    PAYUEL_OBC_Data.PipeDepth = PAYUEL_OBC_PIPE_DEPTH;
    PAYUEL_OBC_Data.ChildTaskId = CFE_ES_TASKID_UNDEFINED;
    PAYUEL_OBC_Data.ChildSemaphore = OS_OBJECT_ID_UNDEFINED;
    PAYUEL_OBC_Data.ChildMutex = OS_OBJECT_ID_UNDEFINED;
    PAYUEL_OBC_Data.ReportMutex = OS_OBJECT_ID_UNDEFINED;
    PAYUEL_OBC_Data.HardwareMutex = OS_OBJECT_ID_UNDEFINED;

    strncpy(PAYUEL_OBC_Data.PipeName, "PAYUEL_OBC_CMD_PIPE", sizeof(PAYUEL_OBC_Data.PipeName));
    PAYUEL_OBC_Data.PipeName[sizeof(PAYUEL_OBC_Data.PipeName) - 1] = 0;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAYUEL_OBC: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_OBC_Data.obc_bcn.TelemetryHeader),
                 CFE_SB_ValueToMsgId(PAYUEL_OBC_OBC_BCN_TLM_MID),
                 sizeof(PAYUEL_OBC_Data.obc_bcn));

    memset(&PAYUEL_OBC_Data.rpt, 0, sizeof(PAYUEL_OBC_Data.rpt));
    CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_OBC_Data.rpt.TelemetryHeader),
                 CFE_SB_ValueToMsgId(PAYUEL_OBC_RPT_TLM_MID),
                 sizeof(PAYUEL_OBC_Data.rpt));

    status = CFE_SB_CreatePipe(&PAYUEL_OBC_Data.CommandPipe, PAYUEL_OBC_Data.PipeDepth, PAYUEL_OBC_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_OBC_SEND_BCN_MID), PAYUEL_OBC_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Error subscribing to SEND_BCN_MID, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    /* Legacy SPI handle init kept commented for reference.
     * PAYUEL_OBC_Data.SpiHandle = CFE_SRL_ApiGetHandle(CFE_SRL_SPIO_HANDLE_INDEXER);
     * if (PAYUEL_OBC_Data.SpiHandle == NULL)
     * {
     *     CFE_EVS_SendEvent(PAYUEL_OBC_INIT_INF_EID, CFE_EVS_EventType_ERROR,
     *                       "PAYUEL_OBC: SPI Handle init failed");
     *     return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
     * }
     */

    /* Protect the shared RPT packet because command handlers and the child task can both publish it. */
    status = OS_MutSemCreate(&PAYUEL_OBC_Data.ReportMutex, PAYUEL_OBC_REPORT_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Report mutex create failed, RC=%ld", (long)status);
        return status;
    }

    /* Serialize payload transactions so normal commands and the full-download child never hit hardware together. */
    status = OS_MutSemCreate(&PAYUEL_OBC_Data.HardwareMutex, PAYUEL_OBC_HARDWARE_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Hardware mutex create failed, RC=%ld", (long)status);
        return status;
    }

    /* Start the background worker used by the aggregate image/sensor download commands. */
    status = PAYUEL_OBC_ChildInit();
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(PAYUEL_OBC_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: Initialized %s", PAYUEL_OBC_VERSION);

    return CFE_SUCCESS;
}
