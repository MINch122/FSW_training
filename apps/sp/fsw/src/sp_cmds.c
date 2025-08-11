#include "sp_task.h"
#include "sp_cmds.h"
#include "sp_msgids.h"
#include "sp_eventids.h"
#include "sp_version.h"
#include "sp_tbl.h"
#include "sp_utils.h"
#include "sp_msg.h"

#include "rpt_interface_cfg.h"

CFE_Status_t SP_APP_SendBcnCmd(const SP_APP_SendBcnCmd_t *Msg) {
    int32 Status;
    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_GPIO_IN_GPIO_INDEXER);

    Status = CFE_SRL_ApiGpioGet(Handle);
    if (Status == 0 || Status == 1) {
        SP_APP_Data.DEPTlm.Payload.get_result = (uint8_t)Status;
    }

    /* If Error, put other value which is not `0` or `1` */
    else SP_APP_Data.DEPTlm.Payload.get_result = 0xFF;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SP_APP_Data.DEPTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SP_APP_Data.DEPTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SP_APP_NoopCmd(const SP_APP_NoopCmd_t *Msg){
    SP_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(SP_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: NOOP command %s", SP_APP_VERSION);

    return CFE_SUCCESS;
}

CFE_Status_t SP_APP_ResetCounterCmd(const SP_APP_ResetCountersCmd_t *Msg){
    SP_APP_Data.CmdCounter = 0;
    SP_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(SP_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: RESET command");

    return CFE_SUCCESS;
}

CFE_Status_t SP_APP_DeployCmd(const SP_APP_DeployCmd_t *Msg){
    SP_APP_Data.CmdCounter++;

    int32 Status;
    
    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_GPIO_OUT_GPIO_INDEXER);
    if (Handle == NULL) return -1;

    Status = CFE_SRL_ApiGpioSet(Handle, Msg->Payload.deploy);
    if (Status != CFE_SUCCESS){
        SP_APP_Data.ErrCounter++;
        CFE_ES_WriteToSysLog("SP: Fail to set value via GPIO : 0x%08lx", (unsigned long)Status);
        return Status;
    }
    
    CFE_EVS_SendEvent(SP_APP_DEPLOY_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "SP : Deply Command Status: 0x%02x \n", Status);
    
    return CFE_SUCCESS;
}

CFE_Status_t SP_APP_Get_DeployCmd(const SP_APP_Get_DeployCmd_t *Msg){
    SP_APP_Data.CmdCounter++;
    
    int32 Status;

    RPT_Report_t Report = {0xFF,};
    Report.MsgID = SP_APP_CMD_MID;
    Report.CommandCode = SP_APP_GET_DEPLOY_CC;
    Report.ReturnDataSize = 1;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_GPIO_IN_GPIO_INDEXER);
    if (Handle == NULL) return -1;
    
    Status = CFE_SRL_ApiGpioGet(Handle);
    
    if(Status == CFE_SRL_BAD_ARGUMENT || Status == CFE_SRL_GPIO_SET_VALUE_ERR){
        SP_APP_Data.ErrCounter++;
        CFE_ES_WriteToSysLog("SP: Fail to get value via GPIO: 0x%08lx", (unsigned long)Status);
        
        Report.ReturnType = RPT_RETTYPE_CFE;
        Report.ReturnCode = Status;
    }
    else {
        Report.ReturnType = RPT_RETTYPE_SUCCESS;
        Report.ReturnCode = CFE_SUCCESS;
        memcpy(Report.ReturnValue, &Status, sizeof(uint8_t));
    }

    /**
     * Transmit Msg to RPT
     */
    // .....

    CFE_EVS_SendEvent(SP_APP_DEPLOY_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: Deploy command: feedback value: 0x%02x \n", Status);
    
    return CFE_SUCCESS;
}