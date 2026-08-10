#include "meow.h"
#include "meow_dispatch.h"
#include "meow_cmds.h"
#include "meow_eventids.h"
#include "meow_msgids.h"
#include "meow_msg.h"

bool MEOW_VerifyCmdLength(const CFE_MSG_Message_t* msg_ptr, size_t expected_len)
{
    size_t            actual_len = 0;
    CFE_SB_MsgId_t    mid        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t cc         = 0;

    CFE_MSG_GetSize(msg_ptr, &actual_len);
    if (expected_len == actual_len)
        return true;

    CFE_MSG_GetMsgId(msg_ptr, &mid);
    CFE_MSG_GetFcnCode(msg_ptr, &cc);
    CFE_EVS_SendEvent(MEOW_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                      "Invalid Msg length: ID=0x%X CC=%u Len=%u Expected=%u",
                      (unsigned)CFE_SB_MsgIdToValue(mid), (unsigned)cc,
                      (unsigned)actual_len, (unsigned)expected_len);
    MEOW_AppData.ErrCounter++;
    return false;
}

void MEOW_ProcessGroundCommand(const CFE_SB_Buffer_t* buf)
{
    CFE_MSG_FcnCode_t cc = 0;
    CFE_MSG_GetFcnCode(&buf->Msg, &cc);
    
/* Dispatch case helper. */
#define DISPATCH(CC, Type, Handler) \
    case CC: \
        if (MEOW_VerifyCmdLength(&buf->Msg, sizeof(Type))) \
            Handler((const Type *)buf); \
        break

    switch (cc) {
        DISPATCH(MEOW_NOOP_CC,             MEOW_NoopCmd_t,          MEOW_NoopCmd);
        DISPATCH(MEOW_RESET_COUNTERS_CC,   MEOW_ResetCountersCmd_t, MEOW_ResetCountersCmd);

        DISPATCH(MEOW_SHELL_EXEC_SYNC_CC,  MEOW_ShellExecSyncCmd_t,  MEOW_ShellExecSyncCmd);
        DISPATCH(MEOW_SHELL_EXEC_ASYNC_CC, MEOW_ShellExecAsyncCmd_t, MEOW_ShellExecAsyncCmd);
        DISPATCH(MEOW_SHELL_POLL_CC,       MEOW_ShellPollCmd_t,      MEOW_ShellPollCmd);
        DISPATCH(MEOW_SHELL_KILL_CC,            MEOW_ShellKillCmd_t,          MEOW_ShellKillCmd);
        DISPATCH(MEOW_SHELL_EXEC_SYNC_FORCE_CC, MEOW_ShellExecSyncForceCmd_t,  MEOW_ShellExecSyncForceCmd);
        DISPATCH(MEOW_SHELL_EXEC_ASYNC_FORCE_CC,MEOW_ShellExecAsyncForceCmd_t, MEOW_ShellExecAsyncForceCmd);

        DISPATCH(MEOW_FILE_READ_CC,        MEOW_FileReadCmd_t,       MEOW_FileReadCmd);
        DISPATCH(MEOW_FILE_WRITE_CC,       MEOW_FileWriteCmd_t,      MEOW_FileWriteCmd);
        DISPATCH(MEOW_FILE_REMOVE_CC,      MEOW_FileRemoveCmd_t,     MEOW_FileRemoveCmd);
        DISPATCH(MEOW_FILE_COPY_CC,        MEOW_FileCopyCmd_t,       MEOW_FileCopyCmd);
        DISPATCH(MEOW_FILE_MOVE_CC,        MEOW_FileMoveCmd_t,       MEOW_FileMoveCmd);
        DISPATCH(MEOW_FILE_STAT_CC,        MEOW_FileStatCmd_t,       MEOW_FileStatCmd);
        DISPATCH(MEOW_FILE_TRUNCATE_CC,    MEOW_FileTruncateCmd_t,   MEOW_FileTruncateCmd);
        DISPATCH(MEOW_FILE_TAIL_CC,        MEOW_FileTailCmd_t,       MEOW_FileTailCmd);
        DISPATCH(MEOW_FILE_CHECKSUM_CC,    MEOW_FileChecksumCmd_t,   MEOW_FileChecksumCmd);
        DISPATCH(MEOW_DISK_STAT_CC,        MEOW_DiskStatCmd_t,       MEOW_DiskStatCmd);

        DISPATCH(MEOW_SYS_SHUTDOWN_CC,     MEOW_SysShutdownCmd_t,    MEOW_SysShutdownCmd);
        DISPATCH(MEOW_SYS_SYNC_CC,         MEOW_SysSyncCmd_t,        MEOW_SysSyncCmd);
        DISPATCH(MEOW_SYS_INFO_CC,         MEOW_SysInfoCmd_t,        MEOW_SysInfoCmd);
        DISPATCH(MEOW_SYS_TIME_GET_CC,     MEOW_SysTimeGetCmd_t,     MEOW_SysTimeGetCmd);
        DISPATCH(MEOW_SYS_TIME_SET_CC,     MEOW_SysTimeSetCmd_t,     MEOW_SysTimeSetCmd);
        DISPATCH(MEOW_SYS_FORCE_KILL_CC,   MEOW_SysForceKillCmd_t,   MEOW_SysForceKillCmd);

#ifdef MEOW_INCLUDE_CSP
        DISPATCH(MEOW_CSP_FTP_UPLOAD_CC,       MEOW_CspFtpUploadCmd_t,       MEOW_CspFtpUploadCmd);
        DISPATCH(MEOW_CSP_FTP_DOWNLOAD_CC,     MEOW_CspFtpDownloadCmd_t,     MEOW_CspFtpDownloadCmd);
#endif /* MEOW_INCLUDE_CSP */

        default:
            CFE_EVS_SendEvent(MEOW_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MEOW: invalid command code CC=%d", cc);
            MEOW_AppData.ErrCounter++;
            break;
    }
#undef DISPATCH
}

void MEOW_TaskPipe(const CFE_SB_Buffer_t* buf)
{
    CFE_SB_MsgId_t mid = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(&buf->Msg, &mid);

    switch (CFE_SB_MsgIdToValue(mid))
    {
        case MEOW_CMD_MID:
            MEOW_ProcessGroundCommand(buf);
            break;

        case MEOW_SEND_HK_MID:
            MEOW_SendHkCmd((const MEOW_SendHkCmd_t *)buf);
            break;

        default:
            CFE_EVS_SendEvent(MEOW_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MEOW: invalid MID=0x%x",
                              (unsigned)CFE_SB_MsgIdToValue(mid));
            break;
    }
}
