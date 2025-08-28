#include "sp_task.h"
#include "sp_cmds.h"
#include "sp_utils.h"
#include "sp_eventids.h"
#include "sp_dispatch.h"

SP_AppData_t SP_AppData;

void SP_AppMain(void){
    CFE_Status_t status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(SP_PERF_ID);

    status = SP_AppInit();
    if (status != CFE_SUCCESS){
        SP_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&SP_AppData.RunStatus) == true){
        CFE_ES_PerfLogExit(SP_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, SP_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(SP_PERF_ID);

        if (status == CFE_SUCCESS){
            SP_TaskPipe(SBBufPtr);
        }
        else{
            CFE_EVS_SendEvent(SP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SP App: SB Pipe Read Error, App Will Exit");

            SP_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(SP_PERF_ID);

    CFE_ES_ExitApp(SP_AppData.RunStatus);
}

CFE_Status_t SP_AppInit(void){
    CFE_Status_t status;

    memset(&SP_AppData, 0, sizeof(SP_AppData));

    SP_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("SP APP: Error Registering Events, RC = 0x%08lx\n", (unsigned long)status);
    }
    else{
        CFE_MSG_Init(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(SP_BCN_TLM_MID), sizeof(SP_AppData.BcnTlm));

        status = CFE_SB_CreatePipe(&SP_AppData.CommandPipe, SP_PIPE_DEPTH, SP_PIPE_NAME);
        
        if (status != CFE_SUCCESS){
            CFE_EVS_SendEvent(SP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SP App: Error creating SB Command Pipe, RC = 0x%08lx\n", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS){
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SP_CMD_MID), SP_AppData.CommandPipe);
        
        if (status != CFE_SUCCESS){
            CFE_EVS_SendEvent(SP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SP APP: Error Subscribing to Cmd MID, RC = 0x%08lx\n", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS){
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SP_SEND_BCN_MID), SP_AppData.CommandPipe);
        
        if (status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(SP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SP APP: Error Subscribing to Send Bcn MID, RC = 0x%08lx\n", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS){

        CFE_EVS_SendEvent(SP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "SP App Successfully Initialized.");
    }

    CFE_SRL_GPIO_Handle_t *In = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_IN_GPIO_INDEXER);
    int Ret = CFE_SRL_ApiGpioGet(In);
    SP_AppData.BcnTlm.IsDeploy = (Ret == 0)? true : false; // `0` indicate Deployed
    

    return status;
}