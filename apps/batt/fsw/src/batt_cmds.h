/**
 * @file
 *   Prototypes for the BATT App Ground Command-handling functions
 */

#ifndef BATT_CMDS_H
#define BATT_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "batt_msg.h"

CFE_Status_t BATT_SendHkCmd(const BATT_SendHkCmd_t *Msg);
CFE_Status_t BATT_NoopCmd(const BATT_NoopCmd_t *Msg);
CFE_Status_t BATT_ResetCountersCmd(const BATT_ResetCountersCmd_t *Msg);
CFE_Status_t BATT_GetHkCmd(const BATT_GetHkCmd_t *Msg);
CFE_Status_t BATT_SetHeaterCmd(const BATT_SetHeaterCmd_t *Msg);
CFE_Status_t BATT_ResetFaultCmd(const BATT_ResetFaultCmd_t *Msg);

#endif /* BATT_CMDS_H */
