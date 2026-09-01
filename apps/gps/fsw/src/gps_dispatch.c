/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 * @file  GPS command pipe dispatch and length verification
 */
#include <stdint.h>

#include "gps_app.h"
#include "gps_dispatch.h"
#include "gps_cmds.h"
#include "gps_eventids.h"
#include "gps_msgids.h"
#include "gps_msg.h"
#include "gps_report.h"
#include "gps_service.h"

#include "gps_cmds_oem.h"

bool GPS_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength == ActualLength)
        return true;

    CFE_MSG_GetMsgId(MsgPtr, &MsgId);
    CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

    CFE_EVS_SendEvent(GPS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                        (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                        (unsigned int)ExpectedLength);


    GPS_AppData.ErrCounter++;

    uint16 lens[2] = {
        (uint16)(ActualLength  > UINT16_MAX ? UINT16_MAX : ActualLength),
        (uint16)(ExpectedLength > UINT16_MAX ? UINT16_MAX : ExpectedLength)
    };

    GPS_SendReport(MsgPtr, lens, sizeof(lens), CFE_STATUS_WRONG_MSG_LENGTH, GPS_MISSION_REPORT_RETTYPE_APP);

    return false;
}

/**
 * One command per line: verify the declared length for the command's own type,
 * then hand the buffer to its handler. Spelling the type out at each case is
 * what keeps the cast checked.
 */
#define GPS_DISPATCH(Code, Type, Handler)                        \
    case Code:                                                   \
        if (GPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(Type)))    \
            Handler((const Type*) SBBufPtr);                     \
        break

void GPS_ProcessDeviceCommand(const CFE_SB_Buffer_t* SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode) {
    GPS_DISPATCH(GPS_OEM_CMD_LOG_CC, GPS_OEM_Cmd_Log_t, GPS_OEM_Cmd_Log);
    GPS_DISPATCH(GPS_OEM_CMD_LOG_ONCE_CC, GPS_OEM_Cmd_LogOnce_t, GPS_OEM_Cmd_LogOnce);
    GPS_DISPATCH(GPS_OEM_CMD_LOG_ONTIME_CC, GPS_OEM_Cmd_LogOnTime_t, GPS_OEM_Cmd_LogOnTime);
    GPS_DISPATCH(GPS_OEM_CMD_LOG_ONCHANGED_CC, GPS_OEM_Cmd_LogOnChanged_t, GPS_OEM_Cmd_LogOnChanged);
    GPS_DISPATCH(GPS_OEM_CMD_LOG_ONNEW_CC, GPS_OEM_Cmd_LogOnNew_t, GPS_OEM_Cmd_LogOnNew);
    GPS_DISPATCH(GPS_OEM_CMD_UNLOG_CC, GPS_OEM_Cmd_Unlog_t, GPS_OEM_Cmd_Unlog);
    GPS_DISPATCH(GPS_OEM_CMD_UNLOGALL_CC, GPS_OEM_Cmd_UnlogAll_t, GPS_OEM_Cmd_UnlogAll);
    GPS_DISPATCH(GPS_OEM_CMD_ELEVATION_CUTOFF_CC, GPS_OEM_Cmd_ElevationCutoff_t, GPS_OEM_Cmd_ElevationCutoff);
    GPS_DISPATCH(GPS_OEM_CMD_INTERFACE_MODE_CC, GPS_OEM_Cmd_InterfaceMode_t, GPS_OEM_Cmd_InterfaceMode);
    GPS_DISPATCH(GPS_OEM_CMD_SERIAL_CONFIG_CC, GPS_OEM_Cmd_SerialConfig_t, GPS_OEM_Cmd_SerialConfig);
    GPS_DISPATCH(GPS_OEM_CMD_PUBLISH_CC, GPS_OEM_Cmd_Publish_t, GPS_OEM_Cmd_Publish);
    GPS_DISPATCH(GPS_OEM_LOG_GET_HANDLER_HK_CC, GPS_OEM_Log_GetHandlerHk_t, GPS_OEM_Log_GetHandlerHk);
    GPS_DISPATCH(GPS_OEM_LOG_GET_STAT_CC, GPS_OEM_Log_GetStat_t, GPS_OEM_Log_GetStat);
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_SET_STATUS_CC, GPS_OEM_Log_HandlerSetStatus_t, GPS_OEM_Log_HandlerSetStatus);
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_GET_STATUS_CC, GPS_OEM_Log_HandlerGetStatus_t, GPS_OEM_Log_HandlerGetStatus);
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_ACTIVATE_CC, GPS_OEM_Log_HandlerActivate_t, GPS_OEM_Log_HandlerActivate);
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_DEACTIVATE_CC, GPS_OEM_Log_HandlerDeactivate_t, GPS_OEM_Log_HandlerDeactivate);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_GO_DORMANT_CC, GPS_OEM_Log_HandlerGoDormant_t, GPS_OEM_Log_HandlerGoDormant);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_WAKEUP_CC, GPS_OEM_Log_HandlerWakeup_t, GPS_OEM_Log_HandlerWakeup);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_ACTIVATE_ALL_CC, GPS_OEM_Log_HandlerActivateAll_t, GPS_OEM_Log_HandlerActivateAll);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_DEACTIVATE_ALL_CC, GPS_OEM_Log_HandlerDeactivateAll_t, GPS_OEM_Log_HandlerDeactivateAll);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_REGISTER_CC, GPS_OEM_Log_HandlerRegister_t, GPS_OEM_Log_HandlerRegister);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_UNREGISTER_CC, GPS_OEM_Log_HandlerUnregister_t, GPS_OEM_Log_HandlerUnregister);
    
    GPS_DISPATCH(GPS_OEM_LOG_ADD_CALLBACK_CC, GPS_OEM_Log_AddCallback_t, GPS_OEM_Log_AddCallback);
    
    GPS_DISPATCH(GPS_OEM_LOG_CLEAR_CALLBACKS_CC, GPS_OEM_Log_ClearCallbacks_t, GPS_OEM_Log_ClearCallbacks);
    
    GPS_DISPATCH(GPS_OEM_LOG_HANDLER_MARK_BROKEN_CC, GPS_OEM_Log_HandlerMarkBroken_t, GPS_OEM_Log_HandlerMarkBroken);
    
    GPS_DISPATCH(GPS_OEM_LOG_GET_MESSAGE_LENGTH_CC, GPS_OEM_Log_GetMessageLength_t, GPS_OEM_Log_GetMessageLength);
    
    GPS_DISPATCH(GPS_OEM_LOG_GET_HANDLER_NAME_CC, GPS_OEM_Log_GetHandlerName_t, GPS_OEM_Log_GetHandlerName);
    
    GPS_DISPATCH(GPS_OEM_LOG_RESET_STAT_CC, GPS_OEM_Log_ResetStat_t, GPS_OEM_Log_ResetStat);
    
    GPS_DISPATCH(GPS_OEM_LOG_GET_RECENT_MESSAGE_CC, GPS_OEM_Log_GetRecentMessage_t, GPS_OEM_Log_GetRecentMessage);
    
    GPS_DISPATCH(GPS_OEM_LOG_REJECT_MISSING_CRC_CC, GPS_OEM_Log_RejectMissingCrc_t, GPS_OEM_Log_RejectMissingCrc);
            
    GPS_DISPATCH(GPS_OEM_LOG_IGNORE_MISSING_CRC_CC, GPS_OEM_Log_IgnoreMissingCrc_t, GPS_OEM_Log_IgnoreMissingCrc);
                
    GPS_DISPATCH(GPS_OEM_LOG_LOCK_HANDLERS_CC, GPS_OEM_Log_LockHandlers_t, GPS_OEM_Log_LockHandlers);
                
    GPS_DISPATCH(GPS_OEM_LOG_UNLOCK_HANDLERS_CC, GPS_OEM_Log_UnlockHandlers_t, GPS_OEM_Log_UnlockHandlers);

    default:
        GPS_AppData.ErrCounter++;
        CFE_EVS_SendEvent(GPS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid device command code: CC = %d",
                            CommandCode);
        GPS_SendReport(&SBBufPtr->Msg, &CommandCode, sizeof(CommandCode),
                       CFE_STATUS_BAD_COMMAND_CODE, GPS_MISSION_REPORT_RETTYPE_APP);
        break;
    }
}

void GPS_ProcessGroundCommand(const CFE_SB_Buffer_t* SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode) {
    GPS_DISPATCH(GPS_NOOP_CC, GPS_NoopCmd_t, GPS_NoopCmd);
    GPS_DISPATCH(GPS_RESET_COUNTERS_CC, GPS_ResetCountersCmd_t, GPS_ResetCountersCmd);
    GPS_DISPATCH(GPS_DRIVER_REPORT_HK_CC, GPS_DriverReportHkCmd_t, GPS_DriverReportHkCmd);

    /* default case already found during FC vs length test */
    default:
        GPS_ProcessDeviceCommand(SBBufPtr);
        break;
    }
}

void GPS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    if (CFE_SB_MsgId_Equal(MsgId, CFE_SB_ValueToMsgId(GPS_SEND_HK_MID)))
        GPS_SendHkCmd((const GPS_SendHkCmd_t *)SBBufPtr);
    else if (CFE_SB_MsgId_Equal(MsgId, CFE_SB_ValueToMsgId(GPS_CMD_MID)))
        GPS_ProcessGroundCommand(SBBufPtr);
    else
        CFE_EVS_SendEvent(GPS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: invalid command packet, MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
}
