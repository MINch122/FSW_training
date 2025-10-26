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
    
        GPS_SendReport(MsgPtr, &ActualLength, sizeof(ActualLength), -1, RPT_RETTYPE_APP); // TODO: define invalid length error.
    }

    return result;
}


void GPS_ProcessDeviceCommand(const CFE_SB_Buffer_t* SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode) {
        case GPS_OEM_CMD_LOG_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_LogCmd_t)))
            {
                GPS_OEMCmd_LogCmd((const GPS_OEMCmd_LogCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONCE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_LogOnceCmd_t)))
            {
                GPS_OEMCmd_LogOnceCmd((const GPS_OEMCmd_LogOnceCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONTIME_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_LogOnTimeCmd_t)))
            {
                GPS_OEMCmd_LogOnTimeCmd((const GPS_OEMCmd_LogOnTimeCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONCHANGED_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_LogOnChangedCmd_t)))
            {
                GPS_OEMCmd_LogOnChangedCmd((const GPS_OEMCmd_LogOnChangedCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_LOG_ONNEW_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_LogOnNewCmd_t)))
            {
                GPS_OEMCmd_LogOnNewCmd((const GPS_OEMCmd_LogOnNewCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_UNLOG_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_UnlogCmd_t)))
            {
                GPS_OEMCmd_UnlogCmd((const GPS_OEMCmd_UnlogCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_UNLOGALL_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_UnlogAllCmd_t)))
            {
                GPS_OEMCmd_UnlogAllCmd((const GPS_OEMCmd_UnlogAllCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_ELEVATION_CUTOFF_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_ElevationCutoffCmd_t)))
            {
                GPS_OEMCmd_ElevationCutoffCmd((const GPS_OEMCmd_ElevationCutoffCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_INTERFACE_MODE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_InterfaceModeCmd_t)))
            {
                GPS_OEMCmd_InterfaceModeCmd((const GPS_OEMCmd_InterfaceModeCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_SERIAL_CONFIG_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_SerialConfigCmd_t)))
            {
                GPS_OEMCmd_SerialConfigCmd((const GPS_OEMCmd_SerialConfigCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_CMD_PUBLISH_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMCmd_PublishCmd_t)))
            {
                GPS_OEMCmd_PublishCmd((const GPS_OEMCmd_PublishCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_GET_HANDLER_HK_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_GetHandlerHkCmd_t)))
            {
                GPS_OEMLog_GetHandlerHkCmd((const GPS_OEMLog_GetHandlerHkCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_GET_MSG_STAT_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_GetMsgStatCmd_t)))
            {
                GPS_OEMLog_GetMsgStatCmd((const GPS_OEMLog_GetMsgStatCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_SET_HANDLER_STATUS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_SetHandlerStatusCmd_t)))
            {
                GPS_OEMLog_SetHandlerStatusCmd((const GPS_OEMLog_SetHandlerStatusCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_GET_HANDLER_STATUS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_GetHandlerStatusCmd_t)))
            {
                GPS_OEMLog_GetHandlerStatusCmd((const GPS_OEMLog_GetHandlerStatusCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_HANDLER_ACTIVATE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerActivateCmd_t)))
            {
                GPS_OEMLog_HandlerActivateCmd((const GPS_OEMLog_HandlerActivateCmd_t*) SBBufPtr);
            }
            break;

        case GPS_OEM_LOG_HANDLER_DEACTIVATE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerDeactivateCmd_t)))
            {
                GPS_OEMLog_HandlerDeactivateCmd((const GPS_OEMLog_HandlerDeactivateCmd_t*) SBBufPtr);
            }
            break;
     
        case GPS_OEM_LOG_HANDLER_GO_DORMANT_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerGoDormantCmd_t)))
            {
                GPS_OEMLog_HandlerGoDormantCmd((const GPS_OEMLog_HandlerGoDormantCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_WAKEUP_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerWakeupCmd_t)))
            {
                GPS_OEMLog_HandlerWakeupCmd((const GPS_OEMLog_HandlerWakeupCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_ACTIVATEALL_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerActivateAllCmd_t)))
            {
                GPS_OEMLog_HandlerActivateAllCmd((const GPS_OEMLog_HandlerActivateAllCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_DEACTIVATEALL_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerDeactivateAllCmd_t)))
            {
                GPS_OEMLog_HandlerDeactivateAllCmd((const GPS_OEMLog_HandlerDeactivateAllCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_REGISTER_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerRegisterCmd_t)))
            {
                GPS_OEMLog_HandlerRegisterCmd((const GPS_OEMLog_HandlerRegisterCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_UNREGISTER_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerUnregisterCmd_t)))
            {
                GPS_OEMLog_HandlerUnregisterCmd((const GPS_OEMLog_HandlerUnregisterCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_ADD_CALLBACK_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_AddCallbackCmd_t)))
            {
                GPS_OEMLog_AddCallbackCmd((const GPS_OEMLog_AddCallbackCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_CLEAR_CALLBACK_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_ClearCallbackCmd_t)))
            {
                GPS_OEMLog_ClearCallbackCmd((const GPS_OEMLog_ClearCallbackCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_HANDLER_SET_BROKEN_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_HandlerSetBrokenCmd_t)))
            {
                GPS_OEMLog_HandlerSetBrokenCmd((const GPS_OEMLog_HandlerSetBrokenCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_GET_MESSAGE_LENGTH_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_GetHandlerMsgLengthCmd_t)))
            {
                GPS_OEMLog_GetHandlerMsgLengthCmd((const GPS_OEMLog_GetHandlerMsgLengthCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_GET_HANDLER_NAME_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_GetHandlerNameCmd_t)))
            {
                GPS_OEMLog_GetHandlerNameCmd((const GPS_OEMLog_GetHandlerNameCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_RESET_HANDLER_COUNTERS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_ResetHandlerCountersCmd_t)))
            {
                GPS_OEMLog_ResetHandlerCountersCmd((const GPS_OEMLog_ResetHandlerCountersCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_DUMP_RECENT_MESSAGE_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_DumpRecentMsgCmd_t)))
            {
                GPS_OEMLog_DumpRecentMsgCmd((const GPS_OEMLog_DumpRecentMsgCmd_t*) SBBufPtr);
            }
            break;
      
        case GPS_OEM_LOG_IGNORE_CHECKSUM_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_IgnoreChecksumCmd_t)))
            {
                GPS_OEMLog_IgnoreChecksumCmd((const GPS_OEMLog_IgnoreChecksumCmd_t*) SBBufPtr);
            }
            break;
             
        case GPS_OEM_LOG_DONOT_IGNORE_CHECKSUM_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_DonotIgnoreChecksumCmd_t)))
            {
                GPS_OEMLog_DonotIgnoreChecksumCmd((const GPS_OEMLog_DonotIgnoreChecksumCmd_t*) SBBufPtr);
            }
            break;
                   
        case GPS_OEM_LOG_LOCK_HANDLERS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_LockHandlersCmd_t)))
            {
                GPS_OEMLog_LockHandlersCmd((const GPS_OEMLog_LockHandlersCmd_t*) SBBufPtr);
            }
            break;
                   
        case GPS_OEM_LOG_UNLOCK_HANDLERS_CC:
            if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_OEMLog_UnlockHandlersCmd_t)))
            {
                GPS_OEMLog_UnlockHandlersCmd((const GPS_OEMLog_UnlockHandlersCmd_t*) SBBufPtr);
            }
            break;

        default:
            GPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(GPS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid device command code: CC = %d",
                              CommandCode);
            GPS_SendReport(SBBufPtr, &CommandCode, sizeof(CommandCode), -1, RPT_RETTYPE_APP); // TODO: define invalid cc error.
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
