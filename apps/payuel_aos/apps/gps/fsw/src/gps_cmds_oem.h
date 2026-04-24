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
#include "cfe_error.h"
#include "gps_msg.h"

/*
** OEM receiver commands.
*/
void GPS_OEMCmd_LogCmd(const GPS_OEMCmd_LogCmd_t* Msg);
void GPS_OEMCmd_LogOnceCmd(const GPS_OEMCmd_LogOnceCmd_t* Msg);
void GPS_OEMCmd_LogOnTimeCmd(const GPS_OEMCmd_LogOnTimeCmd_t* Msg);
void GPS_OEMCmd_LogOnChangedCmd(const GPS_OEMCmd_LogOnChangedCmd_t* Msg);
void GPS_OEMCmd_LogOnNewCmd(const GPS_OEMCmd_LogOnNewCmd_t* Msg);
void GPS_OEMCmd_UnlogCmd(const GPS_OEMCmd_UnlogCmd_t* Msg);
void GPS_OEMCmd_UnlogAllCmd(const GPS_OEMCmd_UnlogAllCmd_t* Msg);
void GPS_OEMCmd_ElevationCutoffCmd(const GPS_OEMCmd_ElevationCutoffCmd_t* Msg);
void GPS_OEMCmd_InterfaceModeCmd(const GPS_OEMCmd_InterfaceModeCmd_t* Msg);
void GPS_OEMCmd_SerialConfigCmd(const GPS_OEMCmd_SerialConfigCmd_t* Msg);
void GPS_OEMCmd_PublishCmd(const GPS_OEMCmd_PublishCmd_t* Msg);

/*
** OEM log handler commands.
*/
void GPS_OEMLog_GetHandlerHkCmd(const GPS_OEMLog_GetHandlerHkCmd_t* Msg);
void GPS_OEMLog_GetMsgStatCmd(const GPS_OEMLog_GetMsgStatCmd_t* Msg);
void GPS_OEMLog_SetHandlerStatusCmd(const GPS_OEMLog_SetHandlerStatusCmd_t* Msg);
void GPS_OEMLog_GetHandlerStatusCmd(const GPS_OEMLog_GetHandlerStatusCmd_t* Msg);
void GPS_OEMLog_HandlerActivateCmd(const GPS_OEMLog_HandlerActivateCmd_t* Msg);
void GPS_OEMLog_HandlerDeactivateCmd(const GPS_OEMLog_HandlerDeactivateCmd_t* Msg);
void GPS_OEMLog_HandlerGoDormantCmd(const GPS_OEMLog_HandlerGoDormantCmd_t* Msg);
void GPS_OEMLog_HandlerWakeupCmd(const GPS_OEMLog_HandlerWakeupCmd_t* Msg);
void GPS_OEMLog_HandlerActivateAllCmd(const GPS_OEMLog_HandlerActivateAllCmd_t* Msg);
void GPS_OEMLog_HandlerDeactivateAllCmd(const GPS_OEMLog_HandlerDeactivateAllCmd_t* Msg);

void GPS_OEMLog_HandlerRegisterCmd(const GPS_OEMLog_HandlerRegisterCmd_t* Msg);
void GPS_OEMLog_HandlerUnregisterCmd(const GPS_OEMLog_HandlerUnregisterCmd_t* Msg);
void GPS_OEMLog_AddCallbackCmd(const GPS_OEMLog_AddCallbackCmd_t* Msg);
void GPS_OEMLog_ClearCallbackCmd(const GPS_OEMLog_ClearCallbackCmd_t* Msg);
void GPS_OEMLog_HandlerSetBrokenCmd(const GPS_OEMLog_HandlerSetBrokenCmd_t* Msg);
void GPS_OEMLog_GetHandlerMsgLengthCmd(const GPS_OEMLog_GetHandlerMsgLengthCmd_t* Msg);
void GPS_OEMLog_GetHandlerNameCmd(const GPS_OEMLog_GetHandlerNameCmd_t* Msg);
void GPS_OEMLog_ResetHandlerCountersCmd(const GPS_OEMLog_ResetHandlerCountersCmd_t* Msg);
void GPS_OEMLog_DumpRecentMsgCmd(const GPS_OEMLog_DumpRecentMsgCmd_t* Msg);
void GPS_OEMLog_IgnoreChecksumCmd(const GPS_OEMLog_IgnoreChecksumCmd_t* Msg);
void GPS_OEMLog_DonotIgnoreChecksumCmd(const GPS_OEMLog_DonotIgnoreChecksumCmd_t* Msg);

void GPS_OEMLog_LockHandlersCmd(const GPS_OEMLog_LockHandlersCmd_t* Msg);
void GPS_OEMLog_UnlockHandlersCmd(const GPS_OEMLog_UnlockHandlersCmd_t* Msg);

#endif
