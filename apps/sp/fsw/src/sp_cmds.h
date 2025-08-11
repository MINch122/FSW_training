#ifndef SP_APP_CMDS_H
#define SP_APP_CMDS_H 

#include "cfe_error.h"
#include "sp_msg.h"

CFE_Status_t SP_APP_ResetCounterCmd(const SP_APP_ResetCountersCmd_t *Msg);
CFE_Status_t SP_APP_NoopCmd(const SP_APP_NoopCmd_t *Msg);
CFE_Status_t SP_APP_DeployCmd(const SP_APP_DeployCmd_t *Msg);
CFE_Status_t SP_APP_Get_DeployCmd(const SP_APP_Get_DeployCmd_t *Msg);

#endif