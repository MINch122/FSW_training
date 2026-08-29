#include "meow.h"
#include "meow_cmds.h"
#include "meow_eventids.h"
#include "meow_version.h"

#include "core/meow_core.h"

#include <string.h>

void MEOW_SendReport(const void* cmd,
                     const void* data,
                     size_t dataSize,
                     int32 retCode,
                     uint8 retType)
{
    CFE_SB_MsgId_t cmdMid;
    CFE_MSG_FcnCode_t cmdCode;
    const uint8 *cursor = (const uint8 *)data;
    size_t offset = 0;
    CFE_Status_t status;
    (void)retType;

    CFE_MSG_GetMsgId(cmd, &cmdMid);
    CFE_MSG_GetFcnCode(cmd, &cmdCode);

    /* A zero-length result still produces one report. Larger results are
     * emitted as consecutive 512-byte RPT-compatible packets.
     * MEOW uses ReturnType as the zero-based chunk index. */
    do
    {
        size_t remaining = dataSize - offset;
        uint16 copySize = (remaining > MEOW_MISSION_MAX_REPORT_LEN)
                              ? MEOW_MISSION_MAX_REPORT_LEN
                              : (uint16)remaining;
        uint8 chunkIndex = (uint8)(offset / MEOW_MISSION_MAX_REPORT_LEN);

        memset(&MEOW_AppData.Report, 0, sizeof(MEOW_AppData.Report));
        status = CFE_MSG_Init(CFE_MSG_PTR(MEOW_AppData.Report.TelemetryHeader),
                              CFE_SB_ValueToMsgId(MEOW_REPORT_TLM_MID),
                              sizeof(MEOW_AppData.Report));
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(MEOW_REPORT_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MEOW: report init failed CC=%u offset=%lu RC=0x%08lX",
                              (unsigned)cmdCode, (unsigned long)offset, (unsigned long)status);
            break;
        }

        MEOW_AppData.Report.Payload.MsgID = (uint16_t)CFE_SB_MsgIdToValue(cmdMid);
        MEOW_AppData.Report.Payload.CommandCode = cmdCode;
        MEOW_AppData.Report.Payload.ReturnType = chunkIndex;
        MEOW_AppData.Report.Payload.ReturnCode = retCode;
        MEOW_AppData.Report.Payload.ReturnDataSize = copySize;
        if (cursor != NULL && copySize > 0)
        {
            memcpy(MEOW_AppData.Report.Payload.ReturnValue, cursor + offset, copySize);
        }

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(MEOW_AppData.Report.TelemetryHeader));
        status = CFE_SB_TransmitMsg(CFE_MSG_PTR(MEOW_AppData.Report.TelemetryHeader), true);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(MEOW_REPORT_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MEOW: report transmit failed CC=%u offset=%lu RC=0x%08lX",
                              (unsigned)cmdCode, (unsigned long)offset, (unsigned long)status);
            break;
        }

        offset += copySize;
    } while (offset < dataSize);
}

/* -------------------------------------------------------------------------
 * Housekeeping
 * ---------------------------------------------------------------------- */

CFE_Status_t MEOW_SendHkCmd(const MEOW_SendHkCmd_t* msg)
{
    MEOW_AppData.HkTlm.Payload.cmd_counter = MEOW_AppData.CmdCounter;
    MEOW_AppData.HkTlm.Payload.err_counter = MEOW_AppData.ErrCounter;
    MEOW_APP_printf("MEOW: HK report requested\n");
    MEOW_SendReport(msg, &MEOW_AppData.HkTlm.Payload, sizeof(MEOW_AppData.HkTlm.Payload), CFE_SUCCESS, 0);
    return CFE_SUCCESS;
}

CFE_Status_t MEOW_NoopCmd(const MEOW_NoopCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    CFE_EVS_SendEvent(MEOW_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "MEOW: NOOP %s", MEOW_VERSION);

    static const char NoopReport[] = "Yosi In Space";
    MEOW_SendReport(msg, NoopReport, sizeof(NoopReport), CFE_SUCCESS, 0);
    return CFE_SUCCESS;
}

CFE_Status_t MEOW_ResetCountersCmd(const MEOW_ResetCountersCmd_t* msg)
{
    MEOW_AppData.CmdCounter = 0;
    MEOW_AppData.ErrCounter = 0;
    CFE_EVS_SendEvent(MEOW_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "MEOW: counters reset");

    uint16 Counters[2] = {MEOW_AppData.CmdCounter, MEOW_AppData.ErrCounter};
    MEOW_SendReport(msg, Counters, sizeof(Counters), CFE_SUCCESS, 0);
    return CFE_SUCCESS;
}

/* -------------------------------------------------------------------------
 * Shell
 * ---------------------------------------------------------------------- */

void MEOW_ShellExecSyncCmd(const MEOW_ShellExecSyncCmd_t* msg)
{
    meow_shell_result_t status = {0};
    MEOW_AppData.CmdCounter++;
    int ret = meow_shell_exec_sync(msg->Payload.cmd,
                                   msg->Payload.redir_path,
                                   msg->Payload.timeout_ms,
                                   &status);
    if (ret == MEOW_SHELL_OK) {
        MEOW_SendReport(msg, &status, sizeof(status), ret, 0);
        if (status.exit_code != 0) {
            CFE_EVS_SendEvent(MEOW_SHELL_EXEC_CODE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MEOW: exec_sync exit %d (pid %d): %.64s",
                              status.exit_code, status.pid, msg->Payload.cmd);
        }
    }
    else {
        struct { meow_shell_result_t result; int32 os_errno; } pack;
        pack.result   = status;
        pack.os_errno = (int32)meow_shell_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &pack, sizeof(pack), ret, 0);
        CFE_EVS_SendEvent(MEOW_SHELL_EXEC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: exec_sync error %d errno=%d: %.64s",
                          ret, (int)pack.os_errno, msg->Payload.cmd);
    }
}

void MEOW_ShellExecAsyncCmd(const MEOW_ShellExecAsyncCmd_t* msg)
{
    int pid = 0;
    MEOW_AppData.CmdCounter++;
    int ret = meow_shell_exec_async(msg->Payload.cmd, msg->Payload.redir_path, &pid);
    if (ret == MEOW_SHELL_OK) {
        MEOW_SendReport(msg, &pid, sizeof(pid), ret, 0);
    }
    else {
        struct { int32 pid; int32 os_errno; } pack;
        pack.pid      = (int32)pid;
        pack.os_errno = (int32)meow_shell_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &pack, sizeof(pack), ret, 0);
        CFE_EVS_SendEvent(MEOW_SHELL_EXEC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: exec_async error %d errno=%d: %.64s",
                          ret, (int)pack.os_errno, msg->Payload.cmd);
    }
}

void MEOW_ShellPollCmd(const MEOW_ShellPollCmd_t* msg)
{
    int exit_code = 0;
    MEOW_AppData.CmdCounter++;
    int ret = meow_shell_poll(msg->Payload.pid, &exit_code);
    if (ret == MEOW_SHELL_OK || ret == MEOW_SHELL_RUNNING) {
        MEOW_SendReport(msg, &exit_code, sizeof(exit_code), ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_shell_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_SHELL_POLL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: shell poll error %d errno=%d pid=%d",
                          ret, (int)os_errno, msg->Payload.pid);
    }
}

void MEOW_ShellKillCmd(const MEOW_ShellKillCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_shell_kill(msg->Payload.pid, msg->Payload.sig);
    if (ret == MEOW_SHELL_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_shell_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_SHELL_KILL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: shell kill error %d errno=%d pid=%d sig=%d",
                          ret, (int)os_errno, msg->Payload.pid, msg->Payload.sig);
    }
}

/* Magic number big-endian interpreter. */
static uint32 read_be32(uint32 raw)
{
    const uint8* b = (const uint8*)&raw;
    return ((uint32)b[0] << 24) | ((uint32)b[1] << 16) |
           ((uint32)b[2] <<  8) |  (uint32)b[3];
}

void MEOW_ShellExecSyncForceCmd(const MEOW_ShellExecSyncForceCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    if (read_be32(msg->Payload.magic) != MEOW_MISSION_SHELL_FORCE_MAGIC) {
        MEOW_AppData.ErrCounter++;
        CFE_EVS_SendEvent(MEOW_SHELL_FORCE_MAGIC_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: force exec rejected — bad magic 0x%08X",
                          (unsigned)read_be32(msg->Payload.magic));
        MEOW_SendReport(msg, &msg->Payload.magic, sizeof(msg->Payload.magic), -1234, 0);
        return;
    }
    meow_shell_result_t res = {0};
    int ret = meow_shell_exec_sync_force(msg->Payload.cmd, msg->Payload.redir_path,
                                         msg->Payload.timeout_ms, &res);
    if (ret == MEOW_SHELL_OK) {
        MEOW_SendReport(msg, &res, sizeof(res), ret, 0);
    }
    else {
        struct { meow_shell_result_t result; int32 os_errno; } pack;
        pack.result   = res;
        pack.os_errno = (int32)meow_shell_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &pack, sizeof(pack), ret, 0);
        CFE_EVS_SendEvent(MEOW_SHELL_EXEC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: force exec_sync error %d errno=%d: %.64s",
                          ret, (int)pack.os_errno, msg->Payload.cmd);
    }
}

void MEOW_ShellExecAsyncForceCmd(const MEOW_ShellExecAsyncForceCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    if (read_be32(msg->Payload.magic) != MEOW_MISSION_SHELL_FORCE_MAGIC) {
        MEOW_AppData.ErrCounter++;
        CFE_EVS_SendEvent(MEOW_SHELL_FORCE_MAGIC_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: force exec rejected — bad magic 0x%08X",
                          (unsigned)read_be32(msg->Payload.magic));
        MEOW_SendReport(msg, &msg->Payload.magic, sizeof(msg->Payload.magic), -1234, 0);
        return;
    }
    int pid = 0;
    int ret = meow_shell_exec_async_force(msg->Payload.cmd, msg->Payload.redir_path, &pid);
    if (ret == MEOW_SHELL_OK) {
        MEOW_SendReport(msg, &pid, sizeof(pid), ret, 0);
    }
    else {
        struct { int32 pid; int32 os_errno; } pack;
        pack.pid      = (int32)pid;
        pack.os_errno = (int32)meow_shell_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &pack, sizeof(pack), ret, 0);
        CFE_EVS_SendEvent(MEOW_SHELL_EXEC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: force exec_async error %d errno=%d: %.64s",
                          ret, (int)pack.os_errno, msg->Payload.cmd);
    }
}

/* -------------------------------------------------------------------------
 * File
 * ---------------------------------------------------------------------- */

void MEOW_FileReadCmd(const MEOW_FileReadCmd_t* msg)
{
    uint8  buf[MEOW_MISSION_MAX_READ_LEN];
    size_t bytes_read = 0;
    uint32 req_size = (msg->Payload.size == 0 || msg->Payload.size > MEOW_MISSION_MAX_READ_LEN)
                      ? MEOW_MISSION_MAX_READ_LEN : msg->Payload.size;
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_read(msg->Payload.path, msg->Payload.offset,
                              buf, req_size, &bytes_read);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, buf, bytes_read, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file read error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_FileWriteCmd(const MEOW_FileWriteCmd_t* msg)
{
    uint16 len = (msg->Payload.data_len > MEOW_MISSION_MAX_WRITE_LEN)
                 ? MEOW_MISSION_MAX_WRITE_LEN : msg->Payload.data_len;
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_write(msg->Payload.path, msg->Payload.data, len,
                               msg->Payload.offset, msg->Payload.flags);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file write error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_FileRemoveCmd(const MEOW_FileRemoveCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_remove(msg->Payload.path);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_REMOVE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file remove error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_FileCopyCmd(const MEOW_FileCopyCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_copy(msg->Payload.src, msg->Payload.dst);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_COPY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file copy error %d errno=%d: %.64s -> %.64s",
                          ret, (int)os_errno, msg->Payload.src, msg->Payload.dst);
    }
}

void MEOW_FileMoveCmd(const MEOW_FileMoveCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_move(msg->Payload.src, msg->Payload.dst);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_MOVE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file move error %d errno=%d: %.64s -> %.64s",
                          ret, (int)os_errno, msg->Payload.src, msg->Payload.dst);
    }
}

void MEOW_FileStatCmd(const MEOW_FileStatCmd_t* msg)
{
    meow_file_stat_t st;
    memset(&st, 0, sizeof(st));
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_stat(msg->Payload.path, &st);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, &st, sizeof(st), ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_STAT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file stat error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_FileTruncateCmd(const MEOW_FileTruncateCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_truncate(msg->Payload.path);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_TRUNC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file truncate error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_FileTailCmd(const MEOW_FileTailCmd_t* msg)
{
    uint8  buf[MEOW_MISSION_MAX_READ_LEN];
    size_t bytes_read = 0;
    uint32 req_size = (msg->Payload.size == 0 || msg->Payload.size > MEOW_MISSION_MAX_READ_LEN)
                      ? MEOW_MISSION_MAX_READ_LEN : msg->Payload.size;
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_tail(msg->Payload.path, msg->Payload.n_bytes,
                              buf, req_size, &bytes_read);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, buf, bytes_read, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_TAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file tail error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_FileChecksumCmd(const MEOW_FileChecksumCmd_t* msg)
{
    uint32 crc = 0;
    MEOW_AppData.CmdCounter++;
    int ret = meow_file_checksum(msg->Payload.path, &crc);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, &crc, sizeof(crc), ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_FILE_CKSUM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: file checksum error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

void MEOW_DiskStatCmd(const MEOW_DiskStatCmd_t* msg)
{
    meow_disk_stat_t ds;
    MEOW_AppData.CmdCounter++;
    int ret = meow_disk_stat(msg->Payload.path, &ds);
    if (ret == MEOW_FILE_OK) {
        MEOW_SendReport(msg, &ds, sizeof(ds), ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_file_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_DISK_STAT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: disk stat error %d errno=%d: %.64s",
                          ret, (int)os_errno, msg->Payload.path);
    }
}

/* -------------------------------------------------------------------------
 * Sys
 * ---------------------------------------------------------------------- */

void MEOW_SysShutdownCmd(const MEOW_SysShutdownCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    if (read_be32(msg->Payload.magic) != MEOW_MISSION_SYS_SHUTDOWN_MAGIC) {
        MEOW_AppData.ErrCounter++;
        CFE_EVS_SendEvent(MEOW_SYS_SHUTDOWN_MAGIC_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: shutdown rejected — bad magic 0x%08X",
                          (unsigned)read_be32(msg->Payload.magic));
        MEOW_SendReport(msg, &msg->Payload.magic, sizeof(msg->Payload.magic), -1234, 0);
        return;
    }
    int ret = meow_sys_shutdown(msg->Payload.type);
    /* only reachable on failure */
    int32 os_errno = (int32)meow_sys_errno();
    MEOW_AppData.ErrCounter++;
    MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
    CFE_EVS_SendEvent(MEOW_SYS_SHUTDOWN_ERR_EID, CFE_EVS_EventType_ERROR,
                      "MEOW: shutdown failed %d errno=%d type=%d",
                      ret, (int)os_errno, msg->Payload.type);
}

void MEOW_SysSyncCmd(const MEOW_SysSyncCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_sys_sync();
    MEOW_SendReport(msg, NULL, 0, ret, 0);
    if (ret != MEOW_SYS_OK) {
        MEOW_AppData.ErrCounter++;
        CFE_EVS_SendEvent(MEOW_SYS_SYNC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: sys sync error %d", ret);
    }
}

void MEOW_SysInfoCmd(const MEOW_SysInfoCmd_t* msg)
{
    meow_sys_info_t info;
    MEOW_AppData.CmdCounter++;
    int ret = meow_sys_info(&info);
    if (ret == MEOW_SYS_OK) {
        MEOW_SendReport(msg, &info, sizeof(info), ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_sys_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_SYS_INFO_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: sys info error %d errno=%d", ret, (int)os_errno);
    }
}

void MEOW_SysTimeGetCmd(const MEOW_SysTimeGetCmd_t* msg)
{
    meow_sys_time_t t;
    MEOW_AppData.CmdCounter++;
    int ret = meow_sys_time_get(&t);
    if (ret == MEOW_SYS_OK) {
        MEOW_SendReport(msg, &t, sizeof(t), ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_sys_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_SYS_TIME_GET_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: sys time_get error %d errno=%d", ret, (int)os_errno);
    }
}

void MEOW_SysTimeSetCmd(const MEOW_SysTimeSetCmd_t* msg)
{
    meow_sys_time_t t;
    t.sec  = (int64_t)msg->Payload.sec;
    t.nsec = (uint32_t)msg->Payload.nsec;
    MEOW_AppData.CmdCounter++;
    int ret = meow_sys_time_set(&t);
    if (ret == MEOW_SYS_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 os_errno = (int32)meow_sys_errno();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
        CFE_EVS_SendEvent(MEOW_SYS_TIME_SET_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: sys time_set error %d errno=%d", ret, (int)os_errno);
    }
}

void MEOW_SysForceKillCmd(const MEOW_SysForceKillCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    if (read_be32(msg->Payload.magic) != MEOW_MISSION_SYS_FORCE_KILL_MAGIC) {
        MEOW_AppData.ErrCounter++;
        CFE_EVS_SendEvent(MEOW_SYS_FORCE_KILL_MAGIC_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: force_kill rejected — bad magic 0x%08X",
                          (unsigned)read_be32(msg->Payload.magic));
        MEOW_SendReport(msg, &msg->Payload.magic, sizeof(msg->Payload.magic), -1234, 0);
        return;
    }
    int ret = meow_sys_force_kill((int)msg->Payload.mode);
    /* meow_sys_force_kill never returns MEOW_SYS_OK */
    int32 os_errno = (int32)meow_sys_errno();
    MEOW_AppData.ErrCounter++;
    MEOW_SendReport(msg, &os_errno, sizeof(os_errno), ret, 0);
    CFE_EVS_SendEvent(MEOW_SYS_FORCE_KILL_ERR_EID, CFE_EVS_EventType_ERROR,
                      "MEOW: force_kill error %d errno=%d mode=%d",
                      ret, (int)os_errno, (int)msg->Payload.mode);
}

#ifdef MEOW_INCLUDE_CSP
/* -------------------------------------------------------------------------
 * CSP
 * ---------------------------------------------------------------------- */

void MEOW_CspFtpUploadCmd(const MEOW_CspFtpUploadCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_csp_ftp_upload(msg->Payload.host,
                                  msg->Payload.port,
                                  msg->Payload.local_path,
                                  msg->Payload.remote_path,
                                  msg->Payload.timeout_ms,
                                  msg->Payload.chunk_size);
    if (ret == MEOW_CSP_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 raw_err = (int32)meow_csp_last_ftp_err();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &raw_err, sizeof(raw_err), ret, 0);
        CFE_EVS_SendEvent(MEOW_CSP_FTP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: csp ftp upload error %d ftp_err=%d: %.64s -> %.64s",
                          ret, (int)raw_err,
                          msg->Payload.local_path, msg->Payload.remote_path);
    }
}

void MEOW_CspFtpDownloadCmd(const MEOW_CspFtpDownloadCmd_t* msg)
{
    MEOW_AppData.CmdCounter++;
    int ret = meow_csp_ftp_download(msg->Payload.host,
                                    msg->Payload.port,
                                    msg->Payload.local_path,
                                    msg->Payload.remote_path,
                                    msg->Payload.timeout_ms,
                                    msg->Payload.chunk_size);
    if (ret == MEOW_CSP_OK) {
        MEOW_SendReport(msg, NULL, 0, ret, 0);
    }
    else {
        int32 raw_err = (int32)meow_csp_last_ftp_err();
        MEOW_AppData.ErrCounter++;
        MEOW_SendReport(msg, &raw_err, sizeof(raw_err), ret, 0);
        CFE_EVS_SendEvent(MEOW_CSP_FTP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MEOW: csp ftp download error %d ftp_err=%d: %.64s -> %.64s",
                          ret, (int)raw_err,
                          msg->Payload.remote_path, msg->Payload.local_path);
    }
}

#endif /* MEOW_INCLUDE_CSP */
