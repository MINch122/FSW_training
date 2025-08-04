#ifndef PAYUZUT_CMDS_H
#define PAYUZUT_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "payuzut_msg.h"

CFE_Status_t PAYUZUT_SendHkCmd(const PAYUZUT_SendHkCmd_t *Msg);
CFE_Status_t PAYUZUT_SendBcnCmd(const PAYUZUT_SendBcnCmd_t *Msg);

CFE_Status_t PAYUZUT_NoopCmd(const PAYUZUT_NoopCmd_t *Msg);
CFE_Status_t PAYUZUT_ResetCountersCmd(const PAYUZUT_ResetCountersCmd_t *Msg);

#endif
