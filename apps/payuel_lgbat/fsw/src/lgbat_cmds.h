#ifndef LGBAT_CMDS_H
#define LGBAT_CMDS_H

#include "cfe.h"
#include "lgbat_msgstruct.h"

CFE_Status_t LGBAT_I2C_ReadBmsData(uint8_t DataID);

void LGBAT_CheckBmsHealth(void);

void LGBAT_SendReport(const CFE_MSG_Message_t *TriggerMsg, uint8_t CC,
                      CFE_Status_t RetCode, const void *Data, uint32_t DataSize);

CFE_Status_t LGBAT_NoopCmd(const LGBAT_NoopCmd_t *Msg);
CFE_Status_t LGBAT_ResetCounterCmd(const LGBAT_ResetCounterCmd_t *Msg);
CFE_Status_t LGBAT_SendBeaconCmd(const CFE_MSG_Message_t *TriggerMsg);
CFE_Status_t LGBAT_RequestDataCmd(const LGBAT_RequestDataCmd_t *Msg);
CFE_Status_t LGBAT_RequestAllDataCmd(const LGBAT_RequestAllDataCmd_t *Msg);
CFE_Status_t LGBAT_SetPowerCmd(const LGBAT_SetPowerCmd_t *Msg);
CFE_Status_t LGBAT_ResetBmsCmd(const LGBAT_ResetBmsCmd_t *Msg);

#endif /* LGBAT_CMDS_H */
