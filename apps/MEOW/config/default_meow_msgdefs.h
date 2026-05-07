/**
 * @file
 *   Specification for the MEOW command and telemetry
 *   message payload and constant definitions.
 */
#ifndef DEFAULT_MEOW_MSGDEFS_H
#define DEFAULT_MEOW_MSGDEFS_H

#include "common_types.h"
#include "meow_fcncodes.h"
#include "meow_interface_cfg.h"

/* -------------------------------------------------------------------------
 * Shell command payloads
 * ---------------------------------------------------------------------- */

typedef struct {
    char   cmd[MEOW_MISSION_MAX_CMD_LEN];
    char   redir_path[MEOW_MISSION_MAX_PATH_LEN];
    uint32 timeout_ms;
    uint8  spare[4];
} MEOW_ShellExecSyncPayload_t;

typedef struct {
    char   cmd[MEOW_MISSION_MAX_CMD_LEN];
    char   redir_path[MEOW_MISSION_MAX_PATH_LEN];
} MEOW_ShellExecAsyncPayload_t;

typedef struct {
    int32  pid;
} MEOW_ShellPollPayload_t;

typedef struct {
    int32  pid;
    int32  sig;
} MEOW_ShellKillPayload_t;

/* force variants — identical to the normal ones but with a magic guard field */
typedef struct {
    char   cmd[MEOW_MISSION_MAX_CMD_LEN];
    char   redir_path[MEOW_MISSION_MAX_PATH_LEN];
    uint32 timeout_ms;
    uint32 magic;  /* must equal MEOW_MISSION_SHELL_FORCE_MAGIC in big-endian */
} MEOW_ShellExecSyncForcePayload_t;

typedef struct {
    char   cmd[MEOW_MISSION_MAX_CMD_LEN];
    char   redir_path[MEOW_MISSION_MAX_PATH_LEN];
    uint32 magic;  /* must equal MEOW_MISSION_SHELL_FORCE_MAGIC in big-endian */
} MEOW_ShellExecAsyncForcePayload_t;

/* -------------------------------------------------------------------------
 * File command payloads
 * ---------------------------------------------------------------------- */

typedef struct {
    char   path[MEOW_MISSION_MAX_PATH_LEN];
    uint32 offset;
    uint32 size;
} MEOW_FileReadPayload_t;

typedef struct {
    char   path[MEOW_MISSION_MAX_PATH_LEN];
    uint32 offset;
    uint16 data_len;
    uint8  flags;
    uint8  spare;
    uint8  data[MEOW_MISSION_MAX_WRITE_LEN];
} MEOW_FileWritePayload_t;

/* used by: remove, stat, truncate, checksum, disk_stat */
typedef struct {
    char   path[MEOW_MISSION_MAX_PATH_LEN];
} MEOW_FilePathPayload_t;

/* used by: copy, move */
typedef struct {
    char   src[MEOW_MISSION_MAX_PATH_LEN];
    char   dst[MEOW_MISSION_MAX_PATH_LEN];
} MEOW_FileSrcDstPayload_t;

typedef struct {
    char   path[MEOW_MISSION_MAX_PATH_LEN];
    uint32 n_bytes;
    uint32 size;
} MEOW_FileTailPayload_t;

/* -------------------------------------------------------------------------
 * Sys command payloads
 * ---------------------------------------------------------------------- */

typedef struct {
    int32  type;   /* MEOW_SYS_SHUTDOWN_* from meow_sys.h */
    uint32 magic;  /* must equal MEOW_MISSION_SYS_SHUTDOWN_MAGIC in big-endian byte order */
} MEOW_SysShutdownPayload_t;

typedef struct {
    uint32 magic;  /* must equal 0xDEADFACE in big-endian byte order */
    int32  mode;   /* meow_sys_force_kill mode (1–5) */
} MEOW_SysForceKillPayload_t;

typedef struct {
    int64  sec;
    uint32 nsec;
    uint8  spare[4];
} MEOW_SysTimeSetPayload_t;

/* -------------------------------------------------------------------------
 * CSP command payloads
 * ---------------------------------------------------------------------- */

typedef struct {
    uint32 timeout_ms;
} MEOW_CspSetReadTimeoutPayload_t;

typedef struct {
    uint8  port;
    char   path[MEOW_MISSION_MAX_PATH_LEN];
    char   symbol[MEOW_MISSION_MAX_SYMBOL_LEN];
} MEOW_CspHandlerLoadPayload_t;

typedef struct {
    uint8  port;
} MEOW_CspHandlerClearPayload_t;

typedef struct {
    uint8  dst;
    uint8  dst_port;
    uint8  src_port;
    uint8  prio;
    uint32 timeout_ms;
    uint16 len;
    uint8  data[MEOW_MISSION_MAX_WRITE_LEN];
} MEOW_CspSendPayload_t;

/* used by: ftp_upload, ftp_download */
typedef struct {
    uint8  host;
    uint8  port;
    uint16 timeout_ms;
    uint16 chunk_size;
    char   local_path[MEOW_MISSION_MAX_PATH_LEN];
    char   remote_path[MEOW_MISSION_MAX_PATH_LEN];
} MEOW_CspFtpPayload_t;

typedef struct {
    char   iface_name[MEOW_MISSION_MAX_IFACE_NAME_LEN];
} MEOW_CspIfstatsPayload_t;

typedef struct {
    uint8  dst;
    uint8  mask;
    uint8  via;
    uint8  spare;
    char   iface_name[MEOW_MISSION_MAX_IFACE_NAME_LEN];
} MEOW_CspRouteSetPayload_t;

typedef struct {
    uint8  idx;
    uint8  dst_port;
    uint8  src_node;
    uint8  fwd_dst;
    uint8  fwd_dst_port;
    uint8  fwd_src_port;
    uint8  active;
    uint8  spare;
    uint16 timeout_ms;
} MEOW_CspRerouteSetPayload_t;

typedef struct {
    uint8  idx;
} MEOW_CspRerouteClearPayload_t;

/* -------------------------------------------------------------------------
 * Telemetry payloads
 * ---------------------------------------------------------------------- */

typedef struct {
    uint16 cmd_counter;
    uint16 err_counter;
} MEOW_HkTlm_Payload_t;

typedef struct {
    uint16 MsgID;
    uint8  CommandCode;
    uint8  ReturnType;
    int32  ReturnCode;
    uint16 ReturnDataSize;
    uint8  ReturnValue[MEOW_MISSION_MAX_READ_LEN];
} MEOW_Report_Payload_t;

#endif /* DEFAULT_MEOW_MSGDEFS_H */
