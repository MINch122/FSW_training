#ifndef LTRX_CMDS_H
#define LTRX_CMDS_H

#include "cfe.h"
#include "ltrx_msg.h"

/* HK / STATUS request handlers */
CFE_Status_t LTRX_SendHkCmd(const CFE_SB_Buffer_t *SBBufPtr);
CFE_Status_t LTRX_SendStatusCmd(const CFE_SB_Buffer_t *SBBufPtr);
void LTRX_SendBcnTlm(void);

/* Basic app management commands */
CFE_Status_t LTRX_NoopCmd(const LTRX_NoopCmd_t *Msg);
CFE_Status_t LTRX_ResetCountersCmd(const LTRX_ResetCountersCmd_t *Msg);
CFE_Status_t LTRX_ResetAppCmdCountersCmd(const LTRX_ResetAppCmdCountersCmd_t *Msg);
CFE_Status_t LTRX_ResetDeviceCmdCountersCmd(const LTRX_ResetDeviceCmdCountersCmd_t *Msg);

/* Session / query commands (CC-based, called from dispatch) */
CFE_Status_t LTRX_SessionStartDownlinkCmd(const LTRX_SessionStartDownlinkCmd_t *Msg);
CFE_Status_t LTRX_SessionAbortCmd(const LTRX_SessionAbortCmd_t *Msg);
CFE_Status_t LTRX_SessionResetStateCmd(const LTRX_SessionResetStateCmd_t *Msg);

CFE_Status_t LTRX_QueryBeaconStatusCmd(const LTRX_QueryBeaconStatusCmd_t *Msg);
CFE_Status_t LTRX_QueryGnssInfoCmd(const LTRX_QueryGnssInfoCmd_t *Msg);

/* Downstream gating commands */
CFE_Status_t LTRX_DownstreamEnableCmd(const LTRX_DownstreamEnableCmd_t *Msg);
CFE_Status_t LTRX_DownstreamDisableCmd(const LTRX_DownstreamDisableCmd_t *Msg);

/* can test */
CFE_Status_t LTRX_TestCspPingCmd(const LTRX_TestCspPingCmd_t *Msg);

#endif /* LTRX_CMDS_H */