/**
 * \file
 *   This file contains the prototypes for the FTP UZURO CAM App Ground Command-handling functions
 */
#ifndef FTP_CMDS_H
#define FTP_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "ftp_msg.h"

CFE_Status_t FTP_SendHkCmd(const FTP_SendHkCmd_t *Msg);
CFE_Status_t FTP_SendBcnCmd(const FTP_SendBcnCmd_t *Msg);

CFE_Status_t FTP_NoopCmd(const FTP_NoopCmd_t *Msg);
CFE_Status_t FTP_ResetCountersCmd(const FTP_ResetCountersCmd_t *Msg);
CFE_Status_t FTP_SendFileCmd(const FTP_SendFileCmd_t *Msg);

#endif