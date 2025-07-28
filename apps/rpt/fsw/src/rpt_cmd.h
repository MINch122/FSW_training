#ifndef RPT_CMD_H
#define RPT_CMD_H

#include "cfe_error.h"
#include "rpt_msg.h"

/**
 * @deprecated RPT only occupy Beacon
 */
CFE_Status_t RPT_SendHKCmd(void);
CFE_Status_t RPT_SendBeaconCmd(void);

CFE_Status_t RPT_NoopCmd(const RPT_NoopCmd_t *Msg);
CFE_Status_t RPT_ResetCounterCmd(const RPT_ResetCounterCmd_t *Msg);
CFE_Status_t RPT_ReportCmd(const RPT_ReportCmd_t *Msg);
CFE_Status_t RPT_ClearQueueCmd(const RPT_ClearQueueCmd_t *Msg);


#endif