#ifndef SP_CMDS_H
#define SP_CMDS_H 

#include "cfe_error.h"
#include "sp_msg.h"

CFE_Status_t SP_ResetCounterCmd(const SP_ResetCountersCmd_t *Msg);
CFE_Status_t SP_NoopCmd(const SP_NoopCmd_t *Msg);
CFE_Status_t SP_DeployCmd(const SP_DeployCmd_t *Msg);
CFE_Status_t SP_Get_DeployCmd(const SP_Get_DeployCmd_t *Msg);

void SP_DeployTask(void);

#endif