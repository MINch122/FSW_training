/**
 * @file
 *   This file contains the prototypes for the PAY UZURO CAM App Ground Command-handling functions
 */
#ifndef PAYUZUC_CMDS_H
#define PAYUZUC_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "payuzuc_msg.h"

CFE_Status_t PAYUZUC_SendHkCmd(const PAYUZUC_SendHkCmd_t *Msg);
CFE_Status_t PAYUZUC_SendBcnCmd(const PAYUZUC_SendBcnCmd_t *Msg);

CFE_Status_t PAYUZUC_NoopCmd(const PAYUZUC_NoopCmd_t *Msg);
CFE_Status_t PAYUZUC_ResetCountersCmd(const PAYUZUC_ResetCountersCmd_t *Msg);

CFE_Status_t PAYUZUC_PingCmd(const PAYUZUC_PingCmd_t *Msg);
CFE_Status_t PAYUZUC_SetModeCmd(const PAYUZUC_SetModeCmd_t *Msg);
CFE_Status_t PAYUZUC_MemoryStatusCmd(const PAYUZUC_MemoryStatusCmd_t *Msg);
CFE_Status_t PAYUZUC_SetExposureCmd(const PAYUZUC_SetExposureCmd_t *Msg);
CFE_Status_t PAYUZUC_CaptureCmd(const PAYUZUC_CaptureCmd_t *Msg);
CFE_Status_t PAYUZUC_DownloadCmd(const PAYUZUC_DownloadCmd_t *Msg);
CFE_Status_t PAYUZUC_ReadRegisterCmd(const PAYUZUC_ReadRegisterCmd_t *Msg);
CFE_Status_t PAYUZUC_WriteRegisterCmd(const PAYUZUC_WriteRegisterCmd_t *Msg);
CFE_Status_t PAYUZUC_DownloadAllCmd(const PAYUZUC_DownloadAllCmd_t *Msg);
CFE_Status_t PAYUZUC_MosaicCmd(const PAYUZUC_MosaicCmd_t *Msg);

#endif /* PAYUZUC_CMDS_H */