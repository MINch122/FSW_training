#include "sp_task.h"
#include "sp_cmds.h"
#include "sp_utils.h"
#include "sp_eventids.h"
#include "sp_dispatch.h"
#include "sp_tbl.h"
#include "sp_version.h"

SP_APP_Data_t SP_APP_Data;

void SP_APP_Main(void){
    CFE_Status_t status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(SP_APP_PERF_ID);

    status = SP_APP_Init();
    if (status != CFE_SUCCESS){
        SP_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&SP_APP_Data.RunStatus) == true){
        CFE_ES_PerfLogExit(SP_APP_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, SP_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(SP_APP_PERF_ID);

        if (status == CFE_SUCCESS){
            SP_APP_TaskPipe(SBBufPtr);
        }
        else{
            CFE_EVS_SendEvent(SP_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SP App: SB Pipe Read Error, App Will Exit");

            SP_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(SP_APP_PERF_ID);

    CFE_ES_ExitApp(SP_APP_Data.RunStatus);
}

CFE_Status_t SP_APP_Init(void){
    CFE_Status_t status;
    char VersionString[SP_APP_CFG_MAX_VERSION_STR_LEN];

    memset(&SP_APP_Data, 0, sizeof(SP_APP_Data));

    SP_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("SP APP: Error Registering Events, RC = 0x%08lx\n", (unsigned long)status);
    }
    else{
        CFE_MSG_Init(CFE_MSG_PTR(SP_APP_Data.DEPTlm.TelemetryHeader), CFE_SB_ValueToMsgId(SP_APP_BCN_TLM_MID), sizeof(SP_APP_Data.DEPTlm));

        status = CFE_SB_CreatePipe(&SP_APP_Data.CommandPipe, SP_APP_PIPE_DEPTH, SP_APP_PIPE_NAME);
        
        if (status != CFE_SUCCESS){
            CFE_EVS_SendEvent(SP_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SP App: Error creating SB Command Pipe, RC = 0x%08lx\n", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS){
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SP_APP_CMD_MID), SP_APP_Data.CommandPipe);
        
        if (status != CFE_SUCCESS){
            CFE_EVS_SendEvent(SP_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SP APP: Error Subscribing to Cmd MID, RC = 0x%08lx\n", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS){
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SP_APP_SEND_BCN_MID), SP_APP_Data.CommandPipe);
        
        if (status != CFE_SUCCESS){
            CFE_EVS_SendEvent(SP_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SP APP: Error Subscribing to Send Bcn MID, RC = 0x%08lx\n", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS){
        status = CFE_TBL_Register(&SP_APP_Data.TblHandles[0], "ExampleTable", sizeof(SP_APP_ExampleTable_t), CFE_TBL_OPT_DEFAULT, SP_APP_TBLValidationFunc);

        if (status != CFE_SUCCESS){
            CFE_EVS_SendEvent(SP_APP_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR, "SP App: Error Registering Example Table, RC = 0x%08lx\n", (unsigned long)status);
        }
        else{
            status = CFE_TBL_Load(SP_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, SP_APP_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, SP_APP_CFG_MAX_VERSION_STR_LEN, "SP App", SP_APP_VERSION, SP_APP_BUILD_CODENAME, SP_APP_LAST_OFFICIAL);

        CFE_EVS_SendEvent(SP_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Sp App Initialized.%s", VersionString);
    }

    return status;
}