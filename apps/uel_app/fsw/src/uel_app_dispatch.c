/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the UEL.
 */

/*
** Include Files:
*/
#include "uel_app.h"
#include "uel_app_dispatch.h"
#include "uel_app_cmds.h"
#include "uel_app_eventids.h"
#include "uel_app_msgids.h"
#include "uel_app_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool UEL_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(UEL_APP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        UEL_APP_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UEL ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void UEL_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process UEL app ground commands
    */
    switch (CommandCode)
    {
      
        case UEL_APP_NOOP_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_SendBcnCmd_t)))
            {
                UEL_APP_NoopCmd((const UEL_APP_NoopCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_SEND_BCN_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_NoopCmd_t)))
            {
                UEL_APP_SendBcnCmd((const UEL_APP_SendBcnCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_RESET_COUNTERS_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_ResetCountersCmd_t)))
            {
                UEL_APP_ResetCountersCmd((const UEL_APP_ResetCountersCmd_t *)SBBufPtr);
            }
            break;


        case UEL_APP_GET_SENS_DATA_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_GetSensDataCmd_t)))
            {
                UEL_APP_GetSensData((const UEL_APP_GetSensDataCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_SET_CAM_POWER_ON_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_SetCamPowerCmd_t)))
            {
                UEL_APP_SetCamPowerOnCmd((const UEL_APP_SetCamPowerCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_SET_CAM_POWER_OFF_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_SetCamPowerCmd_t)))
            {
                UEL_APP_SetCamPowerOffCmd((const UEL_APP_SetCamPowerCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_SET_CAM_SHOT_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_SetCamShotCmd_t)))
            {
                UEL_APP_SetCamShotCmd((const UEL_APP_SetCamShotCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_GET_CAM_SHOT_STATUS_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_GetCamShootStatusCmd_t)))
            {
                UEL_APP_GetCamShotStatus((const UEL_APP_GetCamShootStatusCmd_t *)SBBufPtr);
            }
            break;


        case UEL_APP_GET_CAM_IMAGE_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_GetCamImageCmd_t)))
            {
                UEL_APP_GetCamImageCmd((const UEL_APP_GetCamImageCmd_t *)SBBufPtr);
            }
            break;

        case UEL_APP_SET_TERMINAL_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_SetTerminalCmd_t)))
            {
                UEL_APP_SetTerminalCmd((const UEL_APP_SetTerminalCmd_t *)SBBufPtr);
            }
            break;

     
        case UEL_APP_SET_MOTOR_MODE_CC:
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_SetMotorMode_t)))
            {
                UEL_APP_SetMotorMode((const UEL_APP_SetMotorMode_t *)SBBufPtr);
            }
            break;

        case UEL_APP_DOWNLOAD_IMG_CC:
            /* Download whole Img */
            if (UEL_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UEL_APP_DownloadAllCmd_t)))
            {
                UEL_APP_DownloadImgCmd((const UEL_APP_DownloadAllCmd_t *Msg));
            }
            break;


        default:
            CFE_EVS_SendEvent(UEL_APP_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UEL_APP: Invalid command code: CC = %u", CommandCode);
            UEL_APP_Data.ErrCounter++;
            break;
    }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the UEL    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void UEL_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case UEL_APP_CMD_MID:
            UEL_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case UEL_APP_SEND_BCN_MID:
            UEL_APP_SendBcnCmd((const UEL_APP_SendBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(UEL_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UEL: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
