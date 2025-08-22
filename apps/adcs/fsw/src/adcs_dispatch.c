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
 *   This file contains the source code for the Adcs App.
 */

/*
** Include Files:
*/
#include "adcs_app.h"
#include "adcs_dispatch.h"
#include "adcs_cmds.h"
#include "adcs_msgids.h"
#include "adcs_msg.h"
#include "adcs_utils.h"
#include "adcs_eventids.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool ADCS_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(ADCS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        ADCS_AppData.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ADCS ground commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void ADCS_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process ADCS app ground commands
    */
    switch (CommandCode)
    {
        case ADCS_NOOP_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_NoopCmd_t)))
            {
                ADCS_NoopCmd((const ADCS_NoopCmd_t *)SBBufPtr);
            }
            break;

        case ADCS_RESET_COUNTERS_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_ResetCountersCmd_t)))
            {
                ADCS_ResetCountersCmd((const ADCS_ResetCountersCmd_t *)SBBufPtr);
            }
            break;
        
        /*
         * ADCS UTILS
        */
        case ADCS_GPIO_ENABLE_HIGH_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GpioEnHighCmd_t)))
            {
                ADCS_EN_HighCmd();
            }            
            break;

        case ADCS_GPIO_ENALBE_LOW_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GpioEnLowCmd_t)))
            {
                ADCS_EN_LowCmd();
            } 
            break;

        case ADCS_GPIO_BOOT_HIGH_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GpioBootHighCmd_t)))
            {
                ADCS_Boot_HighCmd();
            } 
            break;
        
        case ADCS_GPIO_BOOT_LOW_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GpioBootLowCmd_t)))
            {
                ADCS_Boot_LowCmd();
            } 
            break;

        case ADCS_EXIT_BOOTLOADER_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_ExitBootLoaderCmd_t)))
            {
                ADCS_ExitBootloader();
            }
            break;

        /* * * * < ADCS Command Code for TC > * * * */
        case ADCS_SET_RESET_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_ResetCmd_t)))
            {
                ADCS_SetReset();
            }
            break;

        case ADCS_SET_CURRENT_UNIX_TIME_CC:
            // ID 2
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_CurrentUnixTimeCmd_t)))
            {
                ADCS_SetCurrentUnixTimeCmd((ADCS_CurrentUnixTimeCmd_t *)SBBufPtr);
            }
            break;
            
        case ADCS_SET_CONTROL_ESTIMATION_MODE_CC:
            // ID 42
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_ControlEstimationModeCmd_t))) {
                ADCS_SetControlEstimationModeCmd((ADCS_ControlEstimationModeCmd_t *)SBBufPtr);
            }

        case ADCS_SET_REFERENCE_LLH_TARGET_CC:
            // ID 48
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_ReferenceLLHTargetCmd_t))) {
                ADCS_SetReferenceLLHTargetCmd((ADCS_ReferenceLLHTargetCmd_t *)SBBufPtr);
            }
        case ADCS_SET_ORBIT_MODE_CC:
            // ID 51
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_OrbitModeCmd_t))) {
                ADCS_SetOrbitModeCmd((ADCS_OrbitModeCmd_t *)SBBufPtr);
            }
            break;
        
        case ADCS_SET_REFERENCE_RPY_VALUES_CC:
            // ID 54
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_ReferenceRPYvaluesCmd_t))) {
                ADCS_SetReferenceRPYValuesCmd((ADCS_ReferenceRPYvaluesCmd_t *)SBBufPtr);
            }
            break;
        
        case ADCS_SET_SAT_ORBIT_PARAMS_CONFIG_CC:
            // ID 68
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_SatOrbitParamConfigCmd_t))) {
                ADCS_SetSatOrbitParamConfigCmd((ADCS_SatOrbitParamConfigCmd_t *)SBBufPtr);
            }
            break;
        
        /* 
        * Process Requested Telemetry
        */
        case ADCS_GET_CURRENT_UNIX_TIME_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetCurrentUnixTimeCmd_t))) {
                ADCS_GetCurrentUnixTimeCmd();
            }
            break;
            
        case ADCS_GET_CONTROL_ESTIMATION_MODE_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetControlEstimationModeCmd_t))) {
                ADCS_GetControlEstimationModeCmd();
            }
            break;
        
        case ADCS_GET_REFERENCE_LLH_TARGET_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetReferenceLLHTargetCmd_t))) {
                ADCS_GetReferenceLLHTargetCmd();
            }
            break;

        case ADCS_GET_ORBIT_MODE_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetOrbitModeCmd_t))) {
                ADCS_GetOrbitModeCmd();
            }
            break;

        case ADCS_GET_RAW_CUBESENSE_SUN_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetRawCubeSenseSunCmd_t))) {
                ADCS_GetRawCubeSenseSunCmd();
            }
            break;

        case ADCS_GET_POWER_STATE_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetPowerStateCmd_t))) {
                ADCS_GetPowerStateCmd();
            }
            break;
        
        case ADCS_GET_SAT_ORBIT_PARAM_CONFIG_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetSatOrbitParamConfigCmd_t))) {
                ADCS_GetSatOrbitParamConfigCmd();
            }
            break;

        case ADCS_GET_RAW_CSS_SENSOR_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetRawCSSSensorCmd_t))) {
                ADCS_GetRawCSSSensorCmd();
            }
            break;

        case ADCS_GET_RAW_GYR_SENSOR_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetRawGYRSensorCmd_t))) {
                ADCS_GetRawGYRSensorCmd();
            }
            break;

        case ADCS_GET_CALIBRATED_GYR_SENSOR_CC:
            if (ADCS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS_GetCalibratedGYRSensorCmd_t))) {
                ADCS_GetCalibratedGYRSensorCmd();
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(ADCS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the ADCS    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void ADCS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ADCS_CMD_MID:
            ADCS_ProcessGroundCommand(SBBufPtr);
            break;

        case ADCS_SEND_HK_MID:
            ADCS_SendHkCmd((const ADCS_SendHkCmd_t *)SBBufPtr);
            break;

        case ADCS_SEND_BCN_MID:
            ADCS_SendBcnCmd((const ADCS_SendBcnCmd_t *)SBBufPtr);
            break;
        
        default:
            CFE_EVS_SendEvent(ADCS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
