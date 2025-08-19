#include "sp_task.h"
#include "sp_cmds.h"
#include "sp_msgids.h"
#include "sp_eventids.h"
#include "sp_utils.h"
#include "sp_msg.h"

#include "rpt_interface_cfg.h"

CFE_Status_t SP_SendBcnCmd(const SP_SendBcnCmd_t *Msg) {
    int32 Status;
    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_IN_GPIO_INDEXER);

    Status = CFE_SRL_ApiGpioGet(Handle);
    if (Status == 0 || Status == 1) {
        SP_AppData.BcnTlm.Payload.get_result = (uint8_t)Status;
    }

    /* If Error, put other value which is not `0` or `1` */
    else SP_AppData.BcnTlm.Payload.get_result = 0xFF;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SP_NoopCmd(const SP_NoopCmd_t *Msg){
    SP_AppData.CmdCounter++;

    CFE_EVS_SendEvent(SP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: NOOP command received.");

    return CFE_SUCCESS;
}

CFE_Status_t SP_ResetCounterCmd(const SP_ResetCountersCmd_t *Msg){
    SP_AppData.CmdCounter = 0;
    SP_AppData.ErrCounter = 0;

    CFE_EVS_SendEvent(SP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: RESET command");

    return CFE_SUCCESS;
}

CFE_Status_t SP_DeployCmd(const SP_DeployCmd_t *Msg){
    SP_AppData.CmdCounter++;

    int32 Status;
    
    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_OUT_GPIO_INDEXER);
    if (Handle == NULL) return -1;

    Status = CFE_SRL_ApiGpioSet(Handle, Msg->Payload.deploy);
    if (Status != CFE_SUCCESS){
        SP_AppData.ErrCounter++;
        CFE_ES_WriteToSysLog("SP: Fail to set value via GPIO : 0x%08X", Status);
    }

    /**
     * Transmit Msg to RPT
     */
    SP_ReportTlm_t *BufPtr = (SP_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(SP_ReportTlm_t));
    if (BufPtr == NULL) return -1;
    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(SP_REPORT_TLM_MID), sizeof(SP_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        goto cleanup;
    }
    BufPtr->Report.MsgID = SP_CMD_MID;
    BufPtr->Report.CommandCode = SP_GET_DEPLOY_CC;
    BufPtr->Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    BufPtr->Report.ReturnCode = Status;
    BufPtr->Report.ReturnDataSize = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        goto cleanup;
    }

cleanup:    
    CFE_EVS_SendEvent(SP_DEPLOY_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "SP : Deply Command Status: 0x%02x \n", Status);
    
    return CFE_SUCCESS;
}

CFE_Status_t SP_Get_DeployCmd(const SP_Get_DeployCmd_t *Msg){
    SP_AppData.CmdCounter++;
    
    int32 Status;
    uint8_t Deploy = 0xFF;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_IN_GPIO_INDEXER);
    if (Handle == NULL) return -1;
    
    Status = CFE_SRL_ApiGpioGet(Handle);
    
    if(Status == CFE_SRL_BAD_ARGUMENT || Status == CFE_SRL_GPIO_SET_VALUE_ERR){
        SP_AppData.ErrCounter++;
        CFE_ES_WriteToSysLog("SP: Fail to get value via GPIO: 0x%08lx", (unsigned long)Status);
    }
    else {
        Deploy = Status;
    }

    /**
     * Transmit Msg to RPT
     */
    SP_ReportTlm_t *BufPtr = (SP_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(SP_ReportTlm_t));
    if (BufPtr == NULL) return -1;
    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(SP_REPORT_TLM_MID), sizeof(SP_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        goto cleanup;
    }
    BufPtr->Report.MsgID = SP_CMD_MID;
    BufPtr->Report.CommandCode = SP_GET_DEPLOY_CC;
    BufPtr->Report.ReturnType = (Status == 0 || Status == 1) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    BufPtr->Report.ReturnCode = (Status == 0 || Status == 1) ? CFE_SUCCESS : Status;
    BufPtr->Report.ReturnDataSize = sizeof(uint8_t);
    memcpy(BufPtr->Report.ReturnValue, &Deploy, sizeof(uint8_t));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        goto cleanup;
    }

cleanup:
    CFE_EVS_SendEvent(SP_DEPLOY_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: Deploy command: feedback value: 0x%02X \n", Status);
    
    return CFE_SUCCESS;
}