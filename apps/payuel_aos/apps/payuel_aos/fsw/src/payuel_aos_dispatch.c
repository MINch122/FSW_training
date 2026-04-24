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
 *   This file contains the source code for the Payuel Aos.
 */

/*
** Include Files:
*/
#include "payuel_aos.h"
#include "payuel_aos_dispatch.h"
#include "payuel_aos_cmds.h"
#include "payuel_aos_eventids.h"
#include "payuel_aos_msgids.h"
#include "payuel_aos_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool PAYUEL_AOS_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(PAYUEL_AOS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        PAYUEL_AOS_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_AOS ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void PAYUEL_AOS_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process PAYUEL_AOS app ground commands
    */
    switch (CommandCode)
    {
        case PAYUEL_AOS_NOOP_CC:
            if (PAYUEL_AOS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_AOS_NoopCmd_t)))
            {
                PAYUEL_AOS_NoopCmd((const PAYUEL_AOS_NoopCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_AOS_RESET_COUNTERS_CC:
            if (PAYUEL_AOS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_AOS_ResetCountersCmd_t)))
            {
                PAYUEL_AOS_ResetCountersCmd((const PAYUEL_AOS_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_AOS_READ_REGISTER_CC:
            if (PAYUEL_AOS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_AOS_Read_RegisterCmd_t)))
            {
                PAYUEL_AOS_Read_RegisterCmd((const PAYUEL_AOS_Read_RegisterCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_AOS_RESET_CC:
            if (PAYUEL_AOS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_AOS_ResetCmd_t)))
            {
                PAYUEL_AOS_ResetCmd((const PAYUEL_AOS_ResetCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_AOS_WRITE_REGISTER_CC:
            if (PAYUEL_AOS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_AOS_Write_RegisterCmd_t)))
            {
                PAYUEL_AOS_Write_RegisterCmd((const PAYUEL_AOS_Write_RegisterCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_AOS_READ_ALL_CHANNELS_CC:
            if (PAYUEL_AOS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_AOS_ReadAllChannelsCmd_t)))
            {
                PAYUEL_AOS_ReadAllChannelsCmd((const PAYUEL_AOS_ReadAllChannelsCmd_t *)SBBufPtr);
            }
            break;


        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(PAYUEL_AOS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the PAYUEL_AOS    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void PAYUEL_AOS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYUEL_AOS_CMD_MID:
            PAYUEL_AOS_ProcessGroundCommand(SBBufPtr);
            break;

        case PAYUEL_AOS_SEND_HK_MID:
            PAYUEL_AOS_SendHkCmd((const PAYUEL_AOS_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_AOS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_AOS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
