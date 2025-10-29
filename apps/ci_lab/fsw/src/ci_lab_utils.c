#include "ci_lab_utils.h"

#include "to_lab_msgids.h"
#include "to_lab_msg.h"

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