#include "payuel_cam.h"
#include "payuel_cam_dispatch.h"
#include "payuel_cam_cmds.h"
#include "payuel_cam_utils.h"
#include "payuel_cam_eventids.h"
#include "payuel_cam_msgids.h"
#include "payuel_cam_msg.h"

void PAYUEL_CAM_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case PAYUEL_CAM_NOOP_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_NoopCmd_t)))
                PAYUEL_CAM_NoopCmd((const PAYUEL_CAM_NoopCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_RESET_COUNTERS_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_ResetCountersCmd_t)))
                PAYUEL_CAM_ResetCountersCmd((const PAYUEL_CAM_ResetCountersCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_SHOT_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_ShotCmd_t)))
                PAYUEL_CAM_ShotCmd((const PAYUEL_CAM_ShotCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_HEALTH_CHECK_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_HealthCheckCmd_t)))
                PAYUEL_CAM_HealthCheckCmd((const PAYUEL_CAM_HealthCheckCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_PROCESS_BINNING_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_ProcessBinningCmd_t)))
                PAYUEL_CAM_ProcessBinningCmd((const PAYUEL_CAM_ProcessBinningCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_DOWNLOAD_META_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_DownloadMetaCmd_t)))
                PAYUEL_CAM_DownloadMetaCmd((const PAYUEL_CAM_DownloadMetaCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_CHUNK_DOWNLOAD_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_ChunkDownloadCmd_t)))
                PAYUEL_CAM_ChunkDownloadCmd((const PAYUEL_CAM_ChunkDownloadCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_DOWNLOAD_IMAGE_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_DownloadImageCmd_t)))
                PAYUEL_CAM_DownloadImageCmd((const PAYUEL_CAM_DownloadImageCmd_t *)SBBufPtr);
            break;

        case PAYUEL_CAM_SEND_BCN_CC:
            if (PAYUEL_CAM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_CAM_SendBcnCmd_t)))
                PAYUEL_CAM_SendBcnCmd((const PAYUEL_CAM_SendBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_CAM_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_CAM: Invalid command code CC=%u", CommandCode);
            PAYUEL_CAM_Data.ErrCounter++;
            break;
    }
}

void PAYUEL_CAM_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYUEL_CAM_CMD_MID:
            PAYUEL_CAM_ProcessGroundCommand(SBBufPtr);
            break;

        case PAYUEL_CAM_SEND_BCN_MID:
            PAYUEL_CAM_SendBcnCmd((const PAYUEL_CAM_SendBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_CAM_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_CAM: Invalid MsgId=0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
