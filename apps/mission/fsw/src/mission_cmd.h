#ifndef MISSION_CMD_H
#define MISSION_CMD_H

#include "cfe_error.h"

CFE_Status_t MISSION_SendHKCmd(const MISSION_SendHkCmd_t *Msg);
CFE_Status_t MISSION_SetCompleteCmd(const MISSION_SetCompleteCmd_t *Msg);
CFE_Status_t MISSION_ReadLeopFileCmd(const MISSION_ReadLeopFileCmd_t *Msg);


#endif
