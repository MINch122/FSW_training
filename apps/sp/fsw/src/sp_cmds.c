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
        SP_AppData.BcnTlm.Payload.DeployStatus = (uint8_t)Status;
        SP_AppData.BcnTlm.IsDeploy = Status ? false : true; // `1` indicate Not deployed
    }

    /* If Error, put other value which is not `0` or `1` */
    else SP_AppData.BcnTlm.Payload.DeployStatus = 0xFF;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SP_NoopCmd(const SP_NoopCmd_t *Msg){
    SP_AppData.CmdCounter++;

    uint8 Cmds[2] = {SP_AppData.CmdCounter, SP_AppData.ErrCounter};

    SP_HandleReport(CFE_SUCCESS, SP_NOOP_CC, Cmds, sizeof(Cmds));

    CFE_EVS_SendEvent(SP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: NOOP command received.");

    return CFE_SUCCESS;
}

CFE_Status_t SP_ResetCounterCmd(const SP_ResetCountersCmd_t *Msg){
    SP_AppData.CmdCounter = 0;
    SP_AppData.ErrCounter = 0;

    uint8 Cmds[2] = {SP_AppData.CmdCounter, SP_AppData.ErrCounter};

    SP_HandleReport(CFE_SUCCESS, SP_RESET_COUNTERS_CC, Cmds, sizeof(Cmds));

    CFE_EVS_SendEvent(SP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: RESET command");

    return CFE_SUCCESS;
}

CFE_Status_t SP_DeployCmd(const SP_DeployCmd_t *Msg){
    SP_AppData.CmdCounter++;

    int32 Status;
    
    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_OUT_GPIO_INDEXER);

    Status = CFE_SRL_ApiGpioSet(Handle, Msg->Payload.deploy);
    if (Status != CFE_SUCCESS){
        SP_AppData.ErrCounter++;
        CFE_ES_WriteToSysLog("SP: Fail to set value via GPIO : 0x%08X", Status);
    }

    /**
     * Transmit Msg to RPT
     */
    SP_HandleReport(Status, SP_DEPLOY_CC, NULL, 0);

    CFE_EVS_SendEvent(SP_DEPLOY_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "SP : Deply Command Status: 0x%02x \n", Status);
    
    return CFE_SUCCESS;
}

CFE_Status_t SP_Get_DeployCmd(const SP_Get_DeployCmd_t *Msg){
    SP_AppData.CmdCounter++;
    
    int32 Status;
    uint8_t Deploy = 0xFF;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_IN_GPIO_INDEXER);
    
    Status = CFE_SRL_ApiGpioGet(Handle);

    if (Status == 0 || Status == 1) {
        Deploy = Status; // Success
        SP_AppData.BcnTlm.IsDeploy = Status ? false : true; // `1` indicate Not deployed
    }
    else { // Fail
        SP_AppData.ErrCounter++;
        CFE_ES_WriteToSysLog("SP: Fail to get value via GPIO: 0x%08lx", (unsigned long)Status);
    }

    /**
     * Transmit Msg to RPT
     */
    SP_HandleReport((Deploy == 0xFF) ? Status : CFE_SUCCESS, SP_GET_DEPLOY_CC, &Deploy, sizeof(Deploy));

    CFE_EVS_SendEvent(SP_DEPLOY_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: Deploy command: feedback value: 0x%02X \n", Status);
    
    return CFE_SUCCESS;
}


CFE_Status_t SP_StartDeployTaskCmd(const SP_StartDeployTaskCmd_t *Msg) {

    CFE_ES_CreateChildTask(&SP_AppData.TaskId, "Deploy Task",
                        SP_DeployTask, CFE_ES_TASK_STACK_ALLOCATE,
                        SP_DEPLOY_TASK_STACK_SIZE, SP_DEPLOY_TASK_STACK_PRIORITY, 0);

    
    return CFE_SUCCESS;
}


void SP_DeployTask(void) {
    SP_AppData.BcnTlm.IsRunning = true;
    SP_AppData.BcnTlm.MaxTry ++;


/*------------------Start of Task-------------------*/
    CFE_SRL_GPIO_Handle_t *Out = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_OUT_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *In = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_IN_GPIO_INDEXER);
    int32 Status;

    CFE_SRL_ApiGpioSet(Out, true);

    do { // Watch the input value, during the Output is `HIGH`
        Status = CFE_SRL_ApiGpioGet(In);
        if (Status == 0)  {
            // If deployed, stop burn & break the loof
            SP_AppData.BcnTlm.IsDeploy = true;
            CFE_SRL_ApiGpioSet(Out, false);
            break; 
        }
        OS_TaskDelay(SP_DEPLOY_TASK_DELAY);
    } while (CFE_SRL_ApiGpioGet(Out) == 1);
    /**
     * If GPIO lines changed to low, stop the loop
     * By Halt RTS or GS command
     */
    /* Drop the gpio to low once again for safety */
    CFE_SRL_ApiGpioSet(Out, false);


/*------------------End of Task-------------------*/
    SP_AppData.BcnTlm.IsRunning = false;
    OS_printf("SP Deploy task done. Is Deployed? %s.\n", SP_AppData.BcnTlm.IsDeploy? "true":"false");
    return;
}