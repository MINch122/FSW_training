#ifndef MEOW_MSGSTRUCT_H
#define MEOW_MSGSTRUCT_H

#include "meow_mission_cfg.h"
#include "meow_msgdefs.h"
#include "cfe_msg_hdr.h"

/* -------------------------------------------------------------------------
 * App commands
 * ---------------------------------------------------------------------- */

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_ResetCountersCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_SendHkCmd_t;


/* -------------------------------------------------------------------------
 * Shell commands
 * ---------------------------------------------------------------------- */

typedef struct {
    CFE_MSG_CommandHeader_t    CommandHeader;
    MEOW_ShellExecSyncPayload_t Payload;
} MEOW_ShellExecSyncCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t     CommandHeader;
    MEOW_ShellExecAsyncPayload_t Payload;
} MEOW_ShellExecAsyncCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_ShellPollPayload_t  Payload;
} MEOW_ShellPollCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_ShellKillPayload_t Payload;
} MEOW_ShellKillCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t          CommandHeader;
    MEOW_ShellExecSyncForcePayload_t Payload;
} MEOW_ShellExecSyncForceCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t           CommandHeader;
    MEOW_ShellExecAsyncForcePayload_t Payload;
} MEOW_ShellExecAsyncForceCmd_t;

/* -------------------------------------------------------------------------
 * File commands
 * ---------------------------------------------------------------------- */

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FileReadPayload_t  Payload;
} MEOW_FileReadCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FileWritePayload_t Payload;
} MEOW_FileWriteCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FilePathPayload_t  Payload;
} MEOW_FileRemoveCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t  CommandHeader;
    MEOW_FileSrcDstPayload_t Payload;
} MEOW_FileCopyCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t  CommandHeader;
    MEOW_FileSrcDstPayload_t Payload;
} MEOW_FileMoveCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FilePathPayload_t  Payload;
} MEOW_FileStatCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FilePathPayload_t  Payload;
} MEOW_FileTruncateCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FileTailPayload_t  Payload;
} MEOW_FileTailCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FilePathPayload_t  Payload;
} MEOW_FileChecksumCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_FilePathPayload_t  Payload;
} MEOW_DiskStatCmd_t;

/* -------------------------------------------------------------------------
 * Sys commands
 * ---------------------------------------------------------------------- */

 typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_SysSyncCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_SysInfoCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_SysTimeGetCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t   CommandHeader;
    MEOW_SysShutdownPayload_t Payload;
} MEOW_SysShutdownCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t    CommandHeader;
    MEOW_SysForceKillPayload_t Payload;
} MEOW_SysForceKillCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t   CommandHeader;
    MEOW_SysTimeSetPayload_t  Payload;
} MEOW_SysTimeSetCmd_t;

/* -------------------------------------------------------------------------
 * CSP commands
 * ---------------------------------------------------------------------- */

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_CspServerStartCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MEOW_CspServerStopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t          CommandHeader;
    MEOW_CspSetReadTimeoutPayload_t  Payload;
} MEOW_CspSetReadTimeoutCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t       CommandHeader;
    MEOW_CspHandlerLoadPayload_t  Payload;
} MEOW_CspHandlerLoadCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t        CommandHeader;
    MEOW_CspHandlerClearPayload_t  Payload;
} MEOW_CspHandlerClearCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_CspSendPayload_t   Payload;
} MEOW_CspSendCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_CspFtpPayload_t    Payload;
} MEOW_CspFtpUploadCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    MEOW_CspFtpPayload_t    Payload;
} MEOW_CspFtpDownloadCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t    CommandHeader;
    MEOW_CspIfstatsPayload_t   Payload;
} MEOW_CspIfstatsCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t    CommandHeader;
    MEOW_CspRouteSetPayload_t  Payload;
} MEOW_CspRouteSetCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t      CommandHeader;
    MEOW_CspRerouteSetPayload_t  Payload;
} MEOW_CspRerouteSetCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t        CommandHeader;
    MEOW_CspRerouteClearPayload_t  Payload;
} MEOW_CspRerouteClearCmd_t;

/* -------------------------------------------------------------------------
 * Telemetry
 * ---------------------------------------------------------------------- */

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    MEOW_HkTlm_Payload_t      Payload;
} MEOW_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    MEOW_Report_Payload_t     Payload;
} MEOW_Report_t;

#endif /* MEOW_MSGSTRUCT_H */
