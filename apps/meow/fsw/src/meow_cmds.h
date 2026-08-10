#ifndef MEOW_CMDS_H
#define MEOW_CMDS_H

#include "cfe_error.h"
#include "meow_msg.h"

/* Housekeeping */
CFE_Status_t MEOW_SendHkCmd(const MEOW_SendHkCmd_t *msg);
CFE_Status_t MEOW_NoopCmd(const MEOW_NoopCmd_t *msg);
CFE_Status_t MEOW_ResetCountersCmd(const MEOW_ResetCountersCmd_t *msg);

/* Shell */
void MEOW_ShellExecSyncCmd(const MEOW_ShellExecSyncCmd_t *msg);
void MEOW_ShellExecAsyncCmd(const MEOW_ShellExecAsyncCmd_t *msg);
void MEOW_ShellPollCmd(const MEOW_ShellPollCmd_t *msg);
void MEOW_ShellKillCmd(const MEOW_ShellKillCmd_t *msg);
void MEOW_ShellExecSyncForceCmd(const MEOW_ShellExecSyncForceCmd_t *msg);
void MEOW_ShellExecAsyncForceCmd(const MEOW_ShellExecAsyncForceCmd_t *msg);

/* File */
void MEOW_FileReadCmd(const MEOW_FileReadCmd_t *msg);
void MEOW_FileWriteCmd(const MEOW_FileWriteCmd_t *msg);
void MEOW_FileRemoveCmd(const MEOW_FileRemoveCmd_t *msg);
void MEOW_FileCopyCmd(const MEOW_FileCopyCmd_t *msg);
void MEOW_FileMoveCmd(const MEOW_FileMoveCmd_t *msg);
void MEOW_FileStatCmd(const MEOW_FileStatCmd_t *msg);
void MEOW_FileTruncateCmd(const MEOW_FileTruncateCmd_t *msg);
void MEOW_FileTailCmd(const MEOW_FileTailCmd_t *msg);
void MEOW_FileChecksumCmd(const MEOW_FileChecksumCmd_t *msg);
void MEOW_DiskStatCmd(const MEOW_DiskStatCmd_t *msg);

/* Sys */
void MEOW_SysShutdownCmd(const MEOW_SysShutdownCmd_t *msg);
void MEOW_SysSyncCmd(const MEOW_SysSyncCmd_t *msg);
void MEOW_SysInfoCmd(const MEOW_SysInfoCmd_t *msg);
void MEOW_SysTimeGetCmd(const MEOW_SysTimeGetCmd_t *msg);
void MEOW_SysTimeSetCmd(const MEOW_SysTimeSetCmd_t *msg);
void MEOW_SysForceKillCmd(const MEOW_SysForceKillCmd_t *msg);

#ifdef MEOW_INCLUDE_CSP
/* CSP */
void MEOW_CspFtpUploadCmd(const MEOW_CspFtpUploadCmd_t *msg);
void MEOW_CspFtpDownloadCmd(const MEOW_CspFtpDownloadCmd_t *msg);
#endif /* MEOW_INCLUDE_CSP */

#endif /* MEOW_CMDS_H */
