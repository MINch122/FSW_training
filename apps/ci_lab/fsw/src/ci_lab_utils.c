#include "ci_lab_utils.h"

#include "to_lab_msgids.h"
#include "to_lab_msg.h"

#include "uant_msgids.h"
#include "uant_msg.h"

#include "hk_msgids.h"
#include "hk_msg.h"
static int32 CI_OpenFile(void) {

    return OS_OpenCreate(&CI_LAB_Global.FileHandle, CI_FILE_NAME, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
}

static int32 CI_ReadFile(void) {
    return OS_read(CI_LAB_Global.FileHandle, &CI_LAB_Global.LastContactTime, sizeof(CI_LAB_Global.LastContactTime));
}

static int32 CI_WriteFile(void) {
    return OS_write(CI_LAB_Global.FileHandle, &CI_LAB_Global.LastContactTime, sizeof(CI_LAB_Global.LastContactTime));
}

static int32 CI_CloseFile(void) {
    return OS_close(CI_LAB_Global.FileHandle);
}


void CI_InitContactTime(void) {
    int32 OsStatus;
    CI_OpenFile();
    OsStatus = CI_ReadFile();
    if (OsStatus == 0) { // If read nothing, this means first file open.
        /* i.e. Initial S/C deployment */
        /* Get CFE_TIME */
        CI_LAB_Global.LastContactTime = CFE_TIME_GetTime();
    }
    CI_CloseFile();
}

void CI_StoreContactTime(void) {
    CI_OpenFile();
    CI_WriteFile();
    CI_CloseFile();
}


void CI_SetEmissionMode(bool IsEmergency) {
    if (IsEmergency) {
        /* Send TO to Dual Transmission */
        TO_SetEmissionModeDualCmd_t Cmd;
        CFE_MSG_Init((CFE_MSG_PTR(Cmd.CommandHeader)), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
        CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_SET_DUAL_EMISSION_CC);

        CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    }
    else {
        /* Send TO to S only Transmission */
        TO_SetEmissionModeSCmd_t Cmd;
        CFE_MSG_Init((CFE_MSG_PTR(Cmd.CommandHeader)), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
        CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_SET_S_ONLY_EMISSION_CC);

        CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    }
}

void CI_SubscribeBeacon(void) {
    TO_LAB_AddPacketCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_LAB_ADD_PKT_CC);
    Cmd.Payload.Stream = CFE_SB_ValueToMsgId(HK_COMBINED_PKT1_MID);
    Cmd.Payload.Flags = (CFE_SB_Qos_t){.Priority = 0, .Reliability = 0};
    Cmd.Payload.BufLimit = 4;

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}


void CI_UnSubscribeBeacon(void) {
    TO_LAB_RemovePacketCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_LAB_REMOVE_PKT_CC);
    Cmd.Payload.Stream = CFE_SB_ValueToMsgId(HK_COMBINED_PKT1_MID);
    
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void CI_UantArm(void) {
    UANT_ISIS_ArmAntennaSystemsCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(UANT_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), UANT_ARM_ANTENNA_SYSTEMS_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void CI_UantDisArm(void) {
    UANT_ISIS_DisarmCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(UANT_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), UANT_DISARM_CC);

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

void CI_UantAutoDeploy(void) {
    /* Sequencial Automate deployment */
    UANT_ISIS_AutomatedDeploymentCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(UANT_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), UANT_AUTOMATED_DEPLOYMENT_CC);
    Cmd.Arg = 5; /* Burn time in sec */

    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}