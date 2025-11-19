/**
 * @file
 *   Report (EO) library
 */
#include "eo_task.h"
#include "common_types.h"
#include "eo_mission_cfg.h"
#include "eo_msgids.h"
#include "eo_utils.h"
#include "eo_file.h"
#include "eo_mission_cfg.h"
#include "eo_eventids.h"
#include "cfe_msgids.h"

#include <unistd.h>
#include <fcntl.h>

void EO_RequestVbattEPS(void) {

    EPS_P31U_GetHkViCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(EPS_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), EPS_P31U_GETHK_VI_INTERNAL_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_RequestSetOutSingle(uint8_t Channel, uint8_t Value) {

    EPS_P31U_SetOutputSingleCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(EPS_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), EPS_P31U_SET_OUT_SINGLE_INTERNAL_CC);
    Cmd.Payload.channel = Channel;
    Cmd.Payload.value = Value;
    Cmd.Payload.delay = 0;
    
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_RequestOutEPS(void) {
    EPS_P31U_GetHkOutCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(EPS_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), EPS_P31U_GETHK_OUT_INTERNAL_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_ExitApps(void) {
    CFE_Status_t Status; 
    /* If Early Orbit Phase done, Exit several apps */
    CFE_ES_AppId_t AppId = CFE_ES_APPID_UNDEFINED;

    /* Delete SANT App */
    Status = CFE_ES_GetAppIDByName(&AppId, "SANT");
    if (Status == CFE_SUCCESS) CFE_ES_DeleteApp(AppId);

    /* Delete SP App */
    Status = CFE_ES_GetAppIDByName(&AppId, "SP");
    if (Status == CFE_SUCCESS) CFE_ES_DeleteApp(AppId);

    /* UANT ? */
    // .....

    /* Exit EO App */
    // CFE_ES_ExitApp(CFE_ES_RunStatus_APP_EXIT);
}

void EO_SantDeploy(void) {
    /* Send Deploy command to SANT */
    SANT_BurnCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CmdHdr), CFE_SB_ValueToMsgId(SANT_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CmdHdr), SANT_BURN_INTERNAL_CC);
    Cmd.Duration = EO_SANT_DURATION;   /* 6 Seconds */

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CmdHdr), true);
}

void EO_TCWait(void) {
    // ??
}


void EO_EnableTO(void) {
    /* Open TO emission */
    TO_LAB_EnableOutputCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_LAB_OUTPUT_ENABLE_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);

}


void EO_EnableBeacon(void) {
    /* Subscribe The HK combined Packet 1 (Beacon, `0x081A)` */
    TO_LAB_AddPacketCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_LAB_ADD_PKT_CC);
    Cmd.Payload.Stream = CFE_SB_ValueToMsgId(HK_COMBINED_PKT1_MID);
    Cmd.Payload.Flags = (CFE_SB_Qos_t){.Priority = 0, .Reliability = 0};
    Cmd.Payload.BufLimit = 4;

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_DisableBeacon(void) {
    /* "Un" Subscribe The HK combined Packet 1 (Beacon, `0x081A)` */
    TO_LAB_RemovePacketCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_LAB_REMOVE_PKT_CC);
    Cmd.Payload.Stream = CFE_SB_ValueToMsgId(HK_COMBINED_PKT1_MID);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_SantConfirm(void) {
    /* Send request deploy command to SANT */
    SANT_SendOpCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(SANT_SEND_OP_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), 0);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_SPDeploy(CFE_SRL_GPIO_Handle_t *Handle, uint8_t Duration) {
    // CFE_SRL_ApiGpioSet(Handle, true);
    OS_TaskDelay(1000 * Duration);
    // CFE_SRL_ApiGpioSet(Handle, false);
}

void EO_RequestMMTDeploy(void) {
    ADCS_MagDeployCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(ADCS_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), ADCS_SET_MAG_DEPLOY_CMD_CC);
    Cmd.Payload.DeployMAG0 = 1;
    Cmd.Payload.DeployMAG1 = 0;
    Cmd.Payload.Spare = 0;

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_RequestMMTTlm(void) {
    ADCS_GetHealthTlmMMTCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(ADCS_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), ADCS_GET_HEALTH_TLM_MMT_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void EO_AdcsDetumble(void) {
    ADCS_SequenceCmdDetumblingCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(ADCS_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), ADCS_SEQ_DTUMB_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}


void EO_EnableRTS2(void) {
    SC_EnableRtsCmd_t Cmd1;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd1.CommandHeader), CFE_SB_ValueToMsgId(SC_CMD_MID), sizeof(Cmd1));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd1.CommandHeader), SC_ENABLE_RTS_CC);
    Cmd1.Payload.RtsNum = 2;
    Cmd1.Payload.Padding = 0;
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd1.CommandHeader), true);
}

void EO_StartRTS2(void) {
    SC_StartRtsCmd_t Cmd1;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd1.CommandHeader), CFE_SB_ValueToMsgId(SC_CMD_MID), sizeof(Cmd1));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd1.CommandHeader), SC_START_RTS_CC);
    Cmd1.Payload.RtsNum = 2;
    Cmd1.Payload.Padding = 0;
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd1.CommandHeader), true);
}



void EO_FinalizePhase(void) {
    /* Write Current Step to File */
    EO_WriteStep();

    /* Debug */
    EO_PRINTF("/-------------Phase Info--------------/\n");
    EO_PRINTF("Phase: %d\n\n", EO_Data.CurrentStep.CurrentPhase);
    EO_PRINTF("S_deploy: %u\n", EO_Data.CurrentStep.S_deploy);
    EO_PRINTF("S_tries: %u\n\n", EO_Data.CurrentStep.S_tries);
    EO_PRINTF("TC received: %u\n\n", EO_Data.CurrentStep.IsTC);
    EO_PRINTF("SP_deploy: %u\n", EO_Data.CurrentStep.SP_deploy);
    EO_PRINTF("SP_tries: %u\n", EO_Data.CurrentStep.SP_tries);
    EO_PRINTF("SP_Sec1: %u\n", EO_Data.CurrentStep.SP_Sec1);
    EO_PRINTF("SP_Sec2: %u\n\n", EO_Data.CurrentStep.SP_Sec2);
    EO_PRINTF("ADCS MMT Deploy: %u\n", EO_Data.MagDeployPinState);
    EO_PRINTF("ADCS MMT Burn: %u\n", EO_Data.MagBurnPinState);
    EO_PRINTF("ADCS MMT Timeout: %u\n", EO_Data.MagDeployTimeout);
    EO_PRINTF("/-------------------------------------/\n");
    
}