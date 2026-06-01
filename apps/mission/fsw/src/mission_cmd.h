#ifndef MISSION_CMD_H
#define MISSION_CMD_H

#include "cfe_error.h"

CFE_Status_t MISSION_SendHKCmd(const MISSION_SendHkCmd_t *Msg);
CFE_Status_t MISSION_SendBeaconCmd(void);

CFE_Status_t MISSION_NoopCmd(const MISSION_NoopCmd_t *Msg);
CFE_Status_t MISSION_ResetCounterCmd(const MISSION_ResetCounterCmd_t *Msg);
CFE_Status_t MISSION_AppsPermOffCmd(const MISSION_AppsPermOffCmd_t*Msg);


#endif
