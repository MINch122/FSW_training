#include "payuel_obc.h"
#include "payuel_obc_dispatch.h"
#include "payuel_obc_cmds.h"
#include "payuel_obc_utils.h"
#include "payuel_obc_eventids.h"
#include "payuel_obc_msgids.h"
#include "payuel_obc_msg.h"

void PAYUEL_OBC_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case PAYUEL_OBC_NOOP_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_NoopCmd_t)))
                PAYUEL_OBC_NoopCmd((const PAYUEL_OBC_NoopCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_RESET_COUNTERS_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_ResetCountersCmd_t)))
                PAYUEL_OBC_ResetCountersCmd((const PAYUEL_OBC_ResetCountersCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_SEND_OBC_BCN_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_SendObcBcnCmd_t)))
                PAYUEL_OBC_SendObcBcnCmd((const PAYUEL_OBC_SendObcBcnCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_MOTOR_MODE_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_MotorModeCmd_t)))
                PAYUEL_OBC_MotorModeCmd((const PAYUEL_OBC_MotorModeCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_CAM_SHOT_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_CamShotCmd_t)))
                PAYUEL_OBC_CamShotCmd((const PAYUEL_OBC_CamShotCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_DOWNLOAD_META_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_DownloadMetaCmd_t)))
                PAYUEL_OBC_DownloadMetaCmd((const PAYUEL_OBC_DownloadMetaCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_CHUNK_DOWNLOAD_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_ChunkDownloadCmd_t)))
                PAYUEL_OBC_ChunkDownloadCmd((const PAYUEL_OBC_ChunkDownloadCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_SENSOR_META_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_SensorMetaCmd_t)))
                PAYUEL_OBC_SensorMetaCmd((const PAYUEL_OBC_SensorMetaCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_SENSOR_CHUNK_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_SensorChunkCmd_t)))
                PAYUEL_OBC_SensorChunkCmd((const PAYUEL_OBC_SensorChunkCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_DOWNLOAD_IMAGE_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_DownloadImageCmd_t)))
                PAYUEL_OBC_DownloadImageCmd((const PAYUEL_OBC_DownloadImageCmd_t *)SBBufPtr);
            break;

        case PAYUEL_OBC_DOWNLOAD_SENSOR_CC:
            if (PAYUEL_OBC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_OBC_DownloadSensorCmd_t)))
                PAYUEL_OBC_DownloadSensorCmd((const PAYUEL_OBC_DownloadSensorCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Invalid command code CC=%u", CommandCode);
            PAYUEL_OBC_Data.ErrCounter++;
            break;
    }
}

void PAYUEL_OBC_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYUEL_OBC_CMD_MID:
            PAYUEL_OBC_ProcessGroundCommand(SBBufPtr);
            break;

        case PAYUEL_OBC_SEND_BCN_MID:
            PAYUEL_OBC_SendObcBcnCmd((const PAYUEL_OBC_SendObcBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_OBC_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Invalid MsgId=0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
