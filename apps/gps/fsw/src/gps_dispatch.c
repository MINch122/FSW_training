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
 *   This file contains the source code for the GPS App.
 */

/*
** Include Files:
*/
#include "gps_app.h"
#include "gps_dispatch.h"
#include "gps_eventids.h"
#include "gps_msgids.h"
#include "gps_msg.h"

#include "gps_cmds.h"
#include "gps_cmds_oem.h"

bool GPS_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(GPS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        GPS_AppData.Counters.ErrCounter++;
    
        GPS_SendReport(MsgPtr, &ActualLength, sizeof(ActualLength), -1, GPS_MISSION_REPORT_RETTYPE_APP); // TODO: define invalid length error.
    }

    return result;
}


void GPS_ProcessDeviceCommand(const CFE_SB_Buffer_t* SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode) {
        case GPS_OEM_CMD_LOG_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_Log_t)))
            {
                GPS_OEM_Cmd_Log((const GPS_OEM_Cmd_Log_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONCE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_LogOnce_t)))
            {
                GPS_OEM_Cmd_LogOnce((const GPS_OEM_Cmd_LogOnce_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONTIME_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_LogOnTime_t)))
            {
                GPS_OEM_Cmd_LogOnTime((const GPS_OEM_Cmd_LogOnTime_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONCHANGED_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_LogOnChanged_t)))
            {
                GPS_OEM_Cmd_LogOnChanged((const GPS_OEM_Cmd_LogOnChanged_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONNEW_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_LogOnNew_t)))
            {
                GPS_OEM_Cmd_LogOnNew((const GPS_OEM_Cmd_LogOnNew_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_UNLOG_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_Unlog_t)))
            {
                GPS_OEM_Cmd_Unlog((const GPS_OEM_Cmd_Unlog_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_UNLOGALL_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_UnlogAll_t)))
            {
                GPS_OEM_Cmd_UnlogAll((const GPS_OEM_Cmd_UnlogAll_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_ELEVATION_CUTOFF_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_ElevationCutoff_t)))
            {
                GPS_OEM_Cmd_ElevationCutoff((const GPS_OEM_Cmd_ElevationCutoff_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_INTERFACE_MODE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_InterfaceMode_t)))
            {
                GPS_OEM_Cmd_InterfaceMode((const GPS_OEM_Cmd_InterfaceMode_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_SERIAL_CONFIG_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_SerialConfig_t)))
            {
                GPS_OEM_Cmd_SerialConfig((const GPS_OEM_Cmd_SerialConfig_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_PUBLISH_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Cmd_Publish_t)))
            {
                GPS_OEM_Cmd_Publish((const GPS_OEM_Cmd_Publish_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_GET_HANDLER_HK_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_GetHandlerHk_t)))
            {
                GPS_OEM_Log_GetHandlerHk((const GPS_OEM_Log_GetHandlerHk_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_GET_STAT_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_GetStat_t)))
            {
                GPS_OEM_Log_GetStat((const GPS_OEM_Log_GetStat_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_HANDLER_SET_STATUS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerSetStatus_t)))
            {
                GPS_OEM_Log_HandlerSetStatus((const GPS_OEM_Log_HandlerSetStatus_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_HANDLER_GET_STATUS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerGetStatus_t)))
            {
                GPS_OEM_Log_HandlerGetStatus((const GPS_OEM_Log_HandlerGetStatus_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_HANDLER_ACTIVATE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerActivate_t)))
            {
                GPS_OEM_Log_HandlerActivate((const GPS_OEM_Log_HandlerActivate_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_HANDLER_DEACTIVATE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerDeactivate_t)))
            {
                GPS_OEM_Log_HandlerDeactivate((const GPS_OEM_Log_HandlerDeactivate_t*) SBBufPtr);
            }
            break;
     
        case GPS_OEM_LOG_HANDLER_GO_DORMANT_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerGoDormant_t)))
            {
                GPS_OEM_Log_HandlerGoDormant((const GPS_OEM_Log_HandlerGoDormant_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_WAKEUP_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerWakeup_t)))
            {
                GPS_OEM_Log_HandlerWakeup((const GPS_OEM_Log_HandlerWakeup_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_ACTIVATE_ALL_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerActivateAll_t)))
            {
                GPS_OEM_Log_HandlerActivateAll((const GPS_OEM_Log_HandlerActivateAll_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_DEACTIVATE_ALL_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerDeactivateAll_t)))
            {
                GPS_OEM_Log_HandlerDeactivateAll((const GPS_OEM_Log_HandlerDeactivateAll_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_REGISTER_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerRegister_t)))
            {
                GPS_OEM_Log_HandlerRegister((const GPS_OEM_Log_HandlerRegister_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_UNREGISTER_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerUnregister_t)))
            {
                GPS_OEM_Log_HandlerUnregister((const GPS_OEM_Log_HandlerUnregister_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_ADD_CALLBACK_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_AddCallback_t)))
            {
                GPS_OEM_Log_AddCallback((const GPS_OEM_Log_AddCallback_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_CLEAR_CALLBACKS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_ClearCallbacks_t)))
            {
                GPS_OEM_Log_ClearCallbacks((const GPS_OEM_Log_ClearCallbacks_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_MARK_BROKEN_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_HandlerMarkBroken_t)))
            {
                GPS_OEM_Log_HandlerMarkBroken((const GPS_OEM_Log_HandlerMarkBroken_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_GET_MESSAGE_LENGTH_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_GetMessageLength_t)))
            {
                GPS_OEM_Log_GetMessageLength((const GPS_OEM_Log_GetMessageLength_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_GET_HANDLER_NAME_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_GetHandlerName_t)))
            {
                GPS_OEM_Log_GetHandlerName((const GPS_OEM_Log_GetHandlerName_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_RESET_STAT_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_ResetStat_t)))
            {
                GPS_OEM_Log_ResetStat((const GPS_OEM_Log_ResetStat_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_GET_RECENT_MESSAGE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_GetRecentMessage_t)))
            {
                GPS_OEM_Log_GetRecentMessage((const GPS_OEM_Log_GetRecentMessage_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_REJECT_MISSING_CRC_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_RejectMissingCrc_t)))
            {
                GPS_OEM_Log_RejectMissingCrc((const GPS_OEM_Log_RejectMissingCrc_t*) SBBufPtr);
            }
            break;
             
        case GPS_OEM_LOG_IGNORE_MISSING_CRC_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_IgnoreMissingCrc_t)))
            {
                GPS_OEM_Log_IgnoreMissingCrc((const GPS_OEM_Log_IgnoreMissingCrc_t*) SBBufPtr);
            }
            break;
                   
        case GPS_OEM_LOG_LOCK_HANDLERS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_LockHandlers_t)))
            {
                GPS_OEM_Log_LockHandlers((const GPS_OEM_Log_LockHandlers_t*) SBBufPtr);
            }
            break;
                   
        case GPS_OEM_LOG_UNLOCK_HANDLERS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEM_Log_UnlockHandlers_t)))
            {
                GPS_OEM_Log_UnlockHandlers((const GPS_OEM_Log_UnlockHandlers_t*) SBBufPtr);
            }
            break;

        default:
            GPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(GPS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid device command code: CC = %d",
                              CommandCode);
            GPS_SendReport(SBBufPtr, &CommandCode, sizeof(CommandCode), -1, GPS_MISSION_REPORT_RETTYPE_APP); // TODO: define invalid cc error.
            break;
    }
}


void GPS_ProcessGroundCommand(const CFE_SB_Buffer_t* SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case GPS_NOOP_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_NoopCmd_t)))
            {
                GPS_NoopCmd((const GPS_NoopCmd_t*) SBBufPtr);
            }
            break;

        case GPS_RESET_COUNTERS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_ResetCountersCmd_t)))
            {
                GPS_ResetCountersCmd((const GPS_ResetCountersCmd_t*) SBBufPtr);
            }
            break;

        /* default case already found during FC vs length test */
        default:
            GPS_ProcessDeviceCommand(SBBufPtr);
            break;
    }
}

void GPS_TaskPipe(const CFE_SB_Buffer_t* SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case GPS_CMD_MID:
            GPS_ProcessGroundCommand(SBBufPtr);
            break;

        case GPS_SEND_HK_MID:
            GPS_SendHkCmd((const GPS_SendHkCmd_t*) SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(GPS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
        // DO NOT REPORT INVALID MSG ID ERRORS.
    }
}
