/**
 * @file
 *   This file contains the prototypes for the PAY UZURO CAM App Ground Command-handling functions
 */
#ifndef paybee_kisscam_CMDS_H
#define paybee_kisscam_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "paybee_kisscam_msg.h"

#define paybee_kisscam_RPT_PHASE_STARTED  1u
#define paybee_kisscam_RPT_PHASE_FINISHED 2u

// CFE_Status_t paybee_kisscam_SendHkCmd(const paybee_kisscam_SendHkCmd_t *Msg);
// CFE_Status_t paybee_kisscam_SendBcnCmd(const paybee_kisscam_SendBcnCmd_t *Msg);

CFE_Status_t paybee_kisscam_NoopCmd(const paybee_kisscam_NoopCmd_t *Msg);
CFE_Status_t paybee_kisscam_ResetCountersCmd(const paybee_kisscam_ResetCountersCmd_t *Msg);

CFE_Status_t paybee_kisscam_PingCmd(const paybee_kisscam_PingCmd_t *Msg);
CFE_Status_t paybee_kisscam_SetModeCmd(const paybee_kisscam_SetModeCmd_t *Msg);
CFE_Status_t paybee_kisscam_MemoryStatusCmd(const paybee_kisscam_MemoryStatusCmd_t *Msg);
CFE_Status_t paybee_kisscam_SetExposureCmd(const paybee_kisscam_SetExposureCmd_t *Msg);
CFE_Status_t paybee_kisscam_CaptureCmd(const paybee_kisscam_CaptureCmd_t *Msg);
CFE_Status_t paybee_kisscam_DownloadCmd(const paybee_kisscam_DownloadCmd_t *Msg);
CFE_Status_t paybee_kisscam_ReadRegisterCmd(const paybee_kisscam_ReadRegisterCmd_t *Msg);
CFE_Status_t paybee_kisscam_WriteRegisterCmd(const paybee_kisscam_WriteRegisterCmd_t *Msg);
CFE_Status_t paybee_kisscam_DownloadAllCmd(const paybee_kisscam_DownloadAllCmd_t *Msg);
CFE_Status_t paybee_kisscam_ImageCompressCmd(const paybee_kisscam_ImageCompressCmd_t *Msg);
// CFE_Status_t paybee_kisscam_DownloadAll2Cmd(const paybee_kisscam_DownloadAllCmd_t *Msg);
// CFE_Status_t paybee_kisscam_MosaicCmd(const paybee_kisscam_MosaicCmd_t *Msg);

#endif /* paybee_kisscam_CMDS_H */
