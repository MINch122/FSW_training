#include "mission_task.h"
#include "mission_eventids.h"
#include "mission_dispatch.h"
#include "mission_utils.h"
#include "utrx_msgids.h"
#include "utrx_msg.h"
#include "cfe_msgids.h"

MISSION_Data_t MISSION_Data;
void MISSION_Main(void) {
    CFE_Status_t Status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(MISSION_PERF_ID);

    Status = MISSION_Init();
    if (Status != CFE_SUCCESS) {
        MISSION_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&MISSION_Data.RunStatus) == true) {
        CFE_ES_PerfLogExit(MISSION_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, MISSION_Data.CmdPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(MISSION_PERF_ID);

        if (Status == CFE_SUCCESS) {
            MISSION_TaskPipe(SBBufPtr);
        }
        else {
            CFE_EVS_SendEvent(MISSION_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "MISSION: SB Pipe Read Error, App will Exit");
            MISSION_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(MISSION_PERF_ID);

    CFE_ES_ExitApp(MISSION_Data.RunStatus);

}

CFE_Status_t MISSION_Init(void) {
    CFE_Status_t Status;

    memset(&MISSION_Data, 0, sizeof(MISSION_Data));
    MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
    MISSION_Data.LEOPFileMutex = OS_OBJECT_ID_UNDEFINED;
    MISSION_Data.LEOPMutex = OS_OBJECT_ID_UNDEFINED;

    Status = OS_MutSemCreate(&MISSION_Data.LEOPFileMutex, "MISSION_LEOP_File", 0);
    if (Status != OS_SUCCESS) {
        CFE_ES_WriteToSysLog("MISSION: Error creating LEOP file mutex. RC = 0x%08lX\n", (unsigned long)Status);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    Status = OS_MutSemCreate(&MISSION_Data.LEOPMutex, "MISSION_LEOP", 0);
    if (Status != OS_SUCCESS) {
        CFE_ES_WriteToSysLog("MISSION: Error creating LEOP mutex. RC = 0x%08lX\n", (unsigned long)Status);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    MISSION_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    MISSION_Data.PipeDepth = MISSION_PIPE_DEPTH;

    strncpy(MISSION_Data.CmdPipeName, "MISSION_CMD_PIPE", sizeof(MISSION_Data.CmdPipeName));
    MISSION_Data.CmdPipeName[sizeof(MISSION_Data.CmdPipeName) - 1] = 0;

    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) { 
        CFE_ES_WriteToSysLog("MISSION: Error Registering Events. RC = 0x%08lX\n", (unsigned long)Status);
    }
    else {
        CFE_MSG_Init(CFE_MSG_PTR(MISSION_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(MISSION_HK_TLM_MID),
                        sizeof(MISSION_Data.HkTlm));
        CFE_MSG_Init(CFE_MSG_PTR(MISSION_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(MISSION_BCN_TLM_MID),
                        sizeof(MISSION_Data.BcnTlm));
        Status = CFE_SB_CreatePipe(&MISSION_Data.CmdPipe, MISSION_Data.PipeDepth, MISSION_Data.CmdPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(MISSION_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MISSION: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MISSION_SEND_HK_MID), MISSION_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(MISSION_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MISSION: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MISSION_SEND_BCN_MID), MISSION_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(MISSION_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MISSION: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MISSION_CMD_MID), MISSION_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(MISSION_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MISSION: Error Subscribing to Cmd request, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UTRX_HK_TLM_MID), MISSION_Data.CmdPipe);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(MISSION_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MISSION: Error Subscribing to UTRX HK tlm, RC = 0x%08lX", (unsigned long)Status);
        }
    }
    if (Status == CFE_SUCCESS) {
        Status = CFE_ES_CreateChildTask(&MISSION_Data.LEOPTaskId, MISSION_LEOP_TASK_NAME, MISSION_LEOP_Task,
                                        CFE_ES_TASK_STACK_ALLOCATE, MISSION_LEOP_TASK_STACK_SIZE,
                                        MISSION_LEOP_TASK_PRIORITY, 0);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(MISSION_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MISSION: Error creating LEOP task, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(MISSION_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "MISSION app Successfully Initialized.\n");
    }

    return Status;
}
