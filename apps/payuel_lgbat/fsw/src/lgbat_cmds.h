#ifndef LGBAT_CMDS_H
#define LGBAT_CMDS_H

#include "cfe.h"
#include "lgbat_msgstruct.h"

// Read one BMS Data ID (0x01-0x0C) from BMS over I2C2.
// Verifies DataID echo, XOR checksum, then parses and caches the data.
CFE_Status_t LGBAT_I2C_ReadBmsData(uint8_t DataID);

// Check BMS health using cached Data09 and Data0A.
// Sends a critical telemetry if any fault or warning is found.
void LGBAT_CheckBmsHealth(void);

// Ground command handlers
CFE_Status_t LGBAT_NoopCmd(const LGBAT_NoopCmd_t *Msg);
CFE_Status_t LGBAT_ResetCounterCmd(const LGBAT_ResetCounterCmd_t *Msg);
CFE_Status_t LGBAT_SendBeaconCmd(void);
CFE_Status_t LGBAT_RequestDataCmd(const LGBAT_RequestDataCmd_t *Msg);
CFE_Status_t LGBAT_RequestAllDataCmd(const LGBAT_RequestAllDataCmd_t *Msg);
CFE_Status_t LGBAT_SetPowerCmd(const LGBAT_SetPowerCmd_t *Msg);
CFE_Status_t LGBAT_ResetBmsCmd(const LGBAT_ResetBmsCmd_t *Msg);

#endif // LGBAT_CMDS_H
