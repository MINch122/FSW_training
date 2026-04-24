#ifndef LGBAT_CMDS_H
#define LGBAT_CMDS_H

#include "cfe.h"
#include "lgbat_msgstruct.h"

// I2C read — called from command handlers and wakeup handler 
CFE_Status_t LGBAT_I2C_ReadBmsData(uint8_t DataID);

// BMS health check — called after every full I2C cycle 
void LGBAT_CheckBmsHealth(void);

// Ground command handlers — each returns CFE_SUCCESS or error code 
CFE_Status_t LGBAT_NoopCmd(const LGBAT_NoopCmd_t *Msg);
CFE_Status_t LGBAT_ResetCounterCmd(const LGBAT_ResetCounterCmd_t *Msg);
CFE_Status_t LGBAT_SendBeaconCmd(void);
CFE_Status_t LGBAT_RequestDataCmd(const LGBAT_RequestDataCmd_t *Msg);
CFE_Status_t LGBAT_RequestAllDataCmd(const LGBAT_RequestAllDataCmd_t *Msg);
CFE_Status_t LGBAT_SetPowerCmd(const LGBAT_SetPowerCmd_t *Msg);
CFE_Status_t LGBAT_ResetBmsCmd(const LGBAT_ResetBmsCmd_t *Msg);

#endif /* LGBAT_CMDS_H */
