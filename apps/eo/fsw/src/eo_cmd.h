/**
 * @file
 *
 * Main header file for the EO Command
 */

#ifndef EO_CMD_H
#define EO_CMD_H

#include "cfe_error.h"
#include "rpt_msg.h"

#include "eps_interface_cfg.h"
#include "adcs_msg.h"

CFE_Status_t EO_SendHKCmd(void);
CFE_Status_t EO_SendBeaconCmd(void);

CFE_Status_t EO_NoopCmd(const EO_NoopCmd_t *Msg);
CFE_Status_t EO_ResetCounterCmd(const EO_ResetCounterCmd_t *Msg);
CFE_Status_t EO_ResetPhaseCmd(const EO_ResetPhaseCmd_t *Msg);
CFE_Status_t EO_NextPhaseCmd(const EO_NextPhaseCmd_t *Msg);
CFE_Status_t EO_FinishPhaseCmd(const EO_FinishPhaseCmd_t *Msg);
CFE_Status_t EO_ExitChildTaskCmd(const EO_ExitChildTaskCmd_t *Msg);
CFE_Status_t EO_StartChildTaskCmd(const EO_StartChildTaskCmd_t *Msg);
CFE_Status_t EO_AppsPermOffCmd(const EO_AppsPermOffCmd_t*Msg);


void EO_WakeupTask(void);

/* External apps data update */
// TODO: Update EO to use P80 EPS telemetry types
// void EO_UpdateDataEPS(const EPS_Vi_Tlm_t *Msg);
// void EO_UpdateOutEPS(const EPS_Output_Tlm_t *Msg);
// void EO_UpdateDataSANT(const SANT_OperationTlm_t *Msg);
void EO_ValidateOperationData(const RPT_OpsTlm_t *Msg);
// void EO_UpdateDataADCS(const ADCS_MMTTlm_t *Msg);


#endif