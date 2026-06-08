#include "ftp_client_app.h"
#include "ftp_client_cmds.h"
#include "ftp_client_dispatch.h"
#include "ftp_client_eventids.h"
#include "ftp_client_fcncodes.h"
#include "ftp_client_msgids.h"

bool FTP_CLIENT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool Result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(FTP_CLIENT_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X, CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId),
                          (unsigned int)FcnCode,
                          (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        FTP_CLIENT_AppData.ErrCounter++;
        Result = false;
    }

    return Result;
}

void FTP_CLIENT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode) {
        case FTP_CLIENT_NOOP_CC:
            if (FTP_CLIENT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(FTP_CLIENT_NoopCmd_t))) {
                FTP_CLIENT_NoopCmd((const FTP_CLIENT_NoopCmd_t *)SBBufPtr);
            }
            break;

        case FTP_CLIENT_UPLOAD_CC:
            if (FTP_CLIENT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(FTP_CLIENT_UploadCmd_t))) {
                FTP_CLIENT_UploadCmd((const FTP_CLIENT_UploadCmd_t *)SBBufPtr);
            }
            break;

        case FTP_CLIENT_DOWNLOAD_CC:
            if (FTP_CLIENT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(FTP_CLIENT_DownloadCmd_t))) {
                FTP_CLIENT_DownloadCmd((const FTP_CLIENT_DownloadCmd_t *)SBBufPtr);
            }
            break;

        default:
            FTP_CLIENT_AppData.ErrCounter++;
            CFE_EVS_SendEvent(FTP_CLIENT_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "FTP_CLIENT: Invalid command code, CC = %u",
                              (unsigned int)CommandCode);
            break;
    }
}

void FTP_CLIENT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId)) {
        case FTP_CLIENT_CMD_MID:
            FTP_CLIENT_ProcessGroundCommand(SBBufPtr);
            break;

        default:
            FTP_CLIENT_AppData.ErrCounter++;
            CFE_EVS_SendEvent(FTP_CLIENT_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "FTP_CLIENT: Invalid message ID, MID = 0x%X",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
