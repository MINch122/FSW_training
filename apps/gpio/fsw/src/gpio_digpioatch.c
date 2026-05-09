/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Gpioace Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the gpioecific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Gpio.
 */

/*
** Include Files:
*/
#include "gpio.h"
#include "gpio_digpioatch.h"
#include "gpio_cmds.h"
#include "gpio_eventids.h"
#include "gpio_msgids.h"
#include "gpio_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool GPIO_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(GPIO_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        GPIO_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void GPIO_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process Gpio ground commands
    */
    switch (CommandCode)
    {
        case GPIO_NOOP_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_NoopCmd_t)))
            {
                GPIO_NoopCmd((const GPIO_NoopCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_RESET_COUNTERS_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_ResetCountersCmd_t)))
            {
                GPIO_ResetCountersCmd((const GPIO_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_PROCESS_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_ProcessCmd_t)))
            {
                GPIO_ProcessCmd((const GPIO_ProcessCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_DIGPIOLAY_PARAM_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_DigpiolayParamCmd_t)))
            {
                GPIO_DigpiolayParamCmd((const GPIO_DigpiolayParamCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_LTRX_EN_ON_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_LtrxEnOnCmd_t)))
            {
                GPIO_LtrxEnOnCmd((const GPIO_LtrxEnOnCmd_t *)SBBufPtr);
            }
            break;
        case GPIO_LTRX_EN_OFF_CC:

            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_LtrxEnOffCmd_t)))
            {
                GPIO_LtrxEnOffCmd((const GPIO_LtrxEnOffCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_DEP1_EN_ON_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_Dep1EnOnCmd_t)))
            {
                GPIO_Dep1EnOnCmd((const GPIO_Dep1EnOnCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_DEP1_EN_OFF_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_Dep1EnOffCmd_t)))
            {
                GPIO_Dep1EnOffCmd((const GPIO_Dep1EnOffCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_DEP2_EN_ON_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_Dep2EnOnCmd_t)))
            {
                GPIO_Dep2EnOnCmd((const GPIO_Dep2EnOnCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_DEP2_EN_OFF_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_Dep2EnOffCmd_t)))
            {
                GPIO_Dep2EnOffCmd((const GPIO_Dep2EnOffCmd_t *)SBBufPtr);
            }
            break;

        case GPIO_SP_IN_READ_5S_CC:
            if (GPIO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPIO_SpInRead5sCmd_t)))
            {
                GPIO_SpInRead5sCmd((const GPIO_SpInRead5sCmd_t *)SBBufPtr);
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void GPIO_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case GPIO_CMD_MID:
            GPIO_ProcessGroundCommand(SBBufPtr);
            break;

        case GPIO_SEND_HK_MID:
            GPIO_SendHkCmd((const GPIO_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(GPIO_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
