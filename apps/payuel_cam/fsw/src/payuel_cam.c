#include "payuel_cam.h"
#include "payuel_cam_cmds.h"
#include "payuel_cam_child.h"
#include "payuel_cam_eventids.h"
#include "payuel_cam_dispatch.h"
#include "payuel_cam_version.h"

PAYUEL_CAM_Data_t PAYUEL_CAM_Data;

void PAYUEL_CAM_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(PAYUEL_CAM_PERF_ID);

    status = PAYUEL_CAM_Init();
    if (status != CFE_SUCCESS)
    {
        PAYUEL_CAM_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&PAYUEL_CAM_Data.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(PAYUEL_CAM_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYUEL_CAM_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(PAYUEL_CAM_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            PAYUEL_CAM_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_CAM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_CAM: SB Pipe Read Error, App Will Exit");
            PAYUEL_CAM_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(PAYUEL_CAM_PERF_ID);
    CFE_ES_ExitApp(PAYUEL_CAM_Data.RunStatus);
}

CFE_Status_t PAYUEL_CAM_Init(void)
{
    CFE_Status_t status;

    memset(&PAYUEL_CAM_Data, 0, sizeof(PAYUEL_CAM_Data));
    PAYUEL_CAM_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    PAYUEL_CAM_Data.PipeDepth = PAYUEL_CAM_PIPE_DEPTH;
    PAYUEL_CAM_Data.ChildTaskId = CFE_ES_TASKID_UNDEFINED;
    PAYUEL_CAM_Data.ChildSemaphore = OS_OBJECT_ID_UNDEFINED;
    PAYUEL_CAM_Data.ChildMutex = OS_OBJECT_ID_UNDEFINED;
    PAYUEL_CAM_Data.ReportMutex = OS_OBJECT_ID_UNDEFINED;
    PAYUEL_CAM_Data.HardwareMutex = OS_OBJECT_ID_UNDEFINED;

    strncpy(PAYUEL_CAM_Data.PipeName, "PAYUEL_CAM_CMD_PIPE", sizeof(PAYUEL_CAM_Data.PipeName));
    PAYUEL_CAM_Data.PipeName[sizeof(PAYUEL_CAM_Data.PipeName) - 1] = 0;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAYUEL_CAM: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_CAM_Data.bcn.TelemetryHeader),
                 CFE_SB_ValueToMsgId(PAYUEL_CAM_BCN_TLM_MID),
                 sizeof(PAYUEL_CAM_Data.bcn));

    memset(&PAYUEL_CAM_Data.rpt, 0, sizeof(PAYUEL_CAM_Data.rpt));
    CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_CAM_Data.rpt.TelemetryHeader),
                 CFE_SB_ValueToMsgId(PAYUEL_CAM_RPT_TLM_MID),
                 sizeof(PAYUEL_CAM_Data.rpt));

    status = CFE_SB_CreatePipe(&PAYUEL_CAM_Data.CommandPipe, PAYUEL_CAM_Data.PipeDepth, PAYUEL_CAM_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_CAM_SEND_BCN_MID), PAYUEL_CAM_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Error subscribing to SEND_BCN_MID, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        return status;
    }

    /* Legacy SPI initialization kept commented for reference.
     * PAYUEL_CAM_Data.SpiHandle = CFE_SRL_ApiGetHandle(CFE_SRL_SPIO_HANDLE_INDEXER);
     * if (PAYUEL_CAM_Data.SpiHandle == NULL)
     * {
     *     CFE_EVS_SendEvent(PAYUEL_CAM_INIT_INF_EID, CFE_EVS_EventType_ERROR,
     *                       "PAYUEL_CAM: SPI Handle init failed");
     *     return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
     * }
     */

    /* Protect the shared RPT packet because command handlers and the child task can both publish it. */
    status = OS_MutSemCreate(&PAYUEL_CAM_Data.ReportMutex, PAYUEL_CAM_REPORT_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Report mutex create failed, RC=%ld", (long)status);
        return status;
    }

    /* Serialize payload transactions so normal commands and the full-download child never hit hardware together. */
    status = OS_MutSemCreate(&PAYUEL_CAM_Data.HardwareMutex, PAYUEL_CAM_HARDWARE_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Hardware mutex create failed, RC=%ld", (long)status);
        return status;
    }

    /* Start the background worker used by the aggregate image download command. */
    status = PAYUEL_CAM_ChildInit();
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(PAYUEL_CAM_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_CAM: Initialized %s", PAYUEL_CAM_VERSION);

    return CFE_SUCCESS;
}
