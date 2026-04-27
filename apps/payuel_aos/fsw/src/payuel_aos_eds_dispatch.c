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

#include "payuel_aos_eds_dispatcher.h"
#include "payuel_aos_eds_dictionary.h"

/*
 * Define a lookup table for PAYUEL_AOS app command codes
 */
static const EdsDispatchTable_PAYUEL_AOS_Application_CFE_SB_Telecommand_t PAYUEL_AOS_TC_DISPATCH_TABLE = {
    .CMD     = {.NoopCmd_indication          = PAYUEL_AOS_NoopCmd,
            .ResetCountersCmd_indication = PAYUEL_AOS_ResetCountersCmd,
            .ProcessCmd_indication       = PAYUEL_AOS_ProcessCmd,
            .DisplayParamCmd_indication  = PAYUEL_AOS_DisplayParamCmd},
    .SEND_HK = {.indication = PAYUEL_AOS_SendHkCmd}};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the PAYUEL_AOS    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void PAYUEL_AOS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t      Status;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_Size_t    MsgSize;
    CFE_MSG_FcnCode_t MsgFc;

    Status = EdsDispatch_PAYUEL_AOS_Application_Telecommand(SBBufPtr, &PAYUEL_AOS_TC_DISPATCH_TABLE);

    if (Status != CFE_SUCCESS)
    {
        CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
        CFE_MSG_GetSize(&SBBufPtr->Msg, &MsgSize);
        CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &MsgFc);
        ++PAYUEL_AOS_Data.ErrCounter;

        if (Status == CFE_STATUS_UNKNOWN_MSG_ID)
        {
            CFE_EVS_SendEvent(PAYUEL_AOS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_AOS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
        }
        else if (Status == CFE_STATUS_WRONG_MSG_LENGTH)
        {
            CFE_EVS_SendEvent(PAYUEL_AOS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)MsgFc, (unsigned int)MsgSize);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_AOS_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_AOS: Invalid ground command code: CC = %d", (int)MsgFc);
        }
    }
}
