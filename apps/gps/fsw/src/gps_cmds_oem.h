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
 * @file
 *   Prototypes for the GPS Application Ground Command-handling functions
 */

#ifndef GPS_CMDS_OEM_H
#define GPS_CMDS_OEM_H

/*
** Required header files.
*/
#include "gps_msg.h"

/*
** OEM receiver commands.
*/
void GPS_OEM_Cmd_Log(const GPS_OEM_Cmd_Log_t* Msg);
void GPS_OEM_Cmd_LogOnce(const GPS_OEM_Cmd_LogOnce_t* Msg);
void GPS_OEM_Cmd_LogOnTime(const GPS_OEM_Cmd_LogOnTime_t* Msg);
void GPS_OEM_Cmd_LogOnChanged(const GPS_OEM_Cmd_LogOnChanged_t* Msg);
void GPS_OEM_Cmd_LogOnNew(const GPS_OEM_Cmd_LogOnNew_t* Msg);
void GPS_OEM_Cmd_Unlog(const GPS_OEM_Cmd_Unlog_t* Msg);
void GPS_OEM_Cmd_UnlogAll(const GPS_OEM_Cmd_UnlogAll_t* Msg);
void GPS_OEM_Cmd_ElevationCutoff(const GPS_OEM_Cmd_ElevationCutoff_t* Msg);
void GPS_OEM_Cmd_InterfaceMode(const GPS_OEM_Cmd_InterfaceMode_t* Msg);
void GPS_OEM_Cmd_SerialConfig(const GPS_OEM_Cmd_SerialConfig_t* Msg);
void GPS_OEM_Cmd_Publish(const GPS_OEM_Cmd_Publish_t* Msg);

/*
** OEM log handler commands.
*/
void GPS_OEM_Log_GetHandlerHk(const GPS_OEM_Log_GetHandlerHk_t* Msg);
void GPS_OEM_Log_GetStat(const GPS_OEM_Log_GetStat_t* Msg);
void GPS_OEM_Log_HandlerSetStatus(const GPS_OEM_Log_HandlerSetStatus_t* Msg);
void GPS_OEM_Log_HandlerGetStatus(const GPS_OEM_Log_HandlerGetStatus_t* Msg);
void GPS_OEM_Log_HandlerActivate(const GPS_OEM_Log_HandlerActivate_t* Msg);
void GPS_OEM_Log_HandlerDeactivate(const GPS_OEM_Log_HandlerDeactivate_t* Msg);
void GPS_OEM_Log_HandlerGoDormant(const GPS_OEM_Log_HandlerGoDormant_t* Msg);
void GPS_OEM_Log_HandlerWakeup(const GPS_OEM_Log_HandlerWakeup_t* Msg);
void GPS_OEM_Log_HandlerActivateAll(const GPS_OEM_Log_HandlerActivateAll_t* Msg);
void GPS_OEM_Log_HandlerDeactivateAll(const GPS_OEM_Log_HandlerDeactivateAll_t* Msg);

void GPS_OEM_Log_HandlerRegister(const GPS_OEM_Log_HandlerRegister_t* Msg);
void GPS_OEM_Log_HandlerUnregister(const GPS_OEM_Log_HandlerUnregister_t* Msg);
void GPS_OEM_Log_AddCallback(const GPS_OEM_Log_AddCallback_t* Msg);
void GPS_OEM_Log_ClearCallbacks(const GPS_OEM_Log_ClearCallbacks_t* Msg);
void GPS_OEM_Log_HandlerMarkBroken(const GPS_OEM_Log_HandlerMarkBroken_t* Msg);
void GPS_OEM_Log_GetMessageLength(const GPS_OEM_Log_GetMessageLength_t* Msg);
void GPS_OEM_Log_GetHandlerName(const GPS_OEM_Log_GetHandlerName_t* Msg);
void GPS_OEM_Log_ResetStat(const GPS_OEM_Log_ResetStat_t* Msg);
void GPS_OEM_Log_GetRecentMessage(const GPS_OEM_Log_GetRecentMessage_t* Msg);
void GPS_OEM_Log_RejectMissingCrc(const GPS_OEM_Log_RejectMissingCrc_t* Msg);
void GPS_OEM_Log_IgnoreMissingCrc(const GPS_OEM_Log_IgnoreMissingCrc_t* Msg);

void GPS_OEM_Log_LockHandlers(const GPS_OEM_Log_LockHandlers_t* Msg);
void GPS_OEM_Log_UnlockHandlers(const GPS_OEM_Log_UnlockHandlers_t* Msg);

#endif
