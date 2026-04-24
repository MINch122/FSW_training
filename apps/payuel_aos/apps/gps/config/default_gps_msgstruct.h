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
 *   Specification for the GPS command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in cosmos_gps_msgdefs.h.
 */
#ifndef DEFAULT_GPS_MSGSTRUCT_H
#define DEFAULT_GPS_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "default_gps_mission_cfg.h"
#include "default_gps_msgdefs.h"
#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

/*************************************************************************/


/**
 * Noarg cmd template.
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} GPS_NoArgCmd_t;

typedef GPS_NoArgCmd_t  GPS_NoopCmd_t;
typedef GPS_NoArgCmd_t  GPS_ResetCountersCmd_t;
typedef GPS_NoArgCmd_t  GPS_GetCountersCmd_t;
typedef GPS_NoArgCmd_t  GPS_GetAppDataCmd_t;
typedef GPS_NoArgCmd_t  GPS_SendHkCmd_t;

/*
** OEM receiver command types.
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_LogCmd_Payload_t Payload;
} GPS_OEMCmd_LogCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_LogOnceCmd_Payload_t Payload;
} GPS_OEMCmd_LogOnceCmd_t;

#define a sizeof(GPS_OEMCmd_LogCmd_t)

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_LogOnTimeCmd_Payload_t Payload;
} GPS_OEMCmd_LogOnTimeCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_LogOnChangedCmd_Payload_t Payload;
} GPS_OEMCmd_LogOnChangedCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_LogOnNewCmd_Payload_t Payload;
} GPS_OEMCmd_LogOnNewCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_UnlogCmd_Payload_t Payload;
} GPS_OEMCmd_UnlogCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_UnlogAllCmd_Payload_t Payload;
} GPS_OEMCmd_UnlogAllCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_ElevationCutoffCmd_Payload_t Payload;
} GPS_OEMCmd_ElevationCutoffCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_InterfaceModeCmd_Payload_t Payload;
} GPS_OEMCmd_InterfaceModeCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_SerialConfigCmd_Payload_t Payload;
} GPS_OEMCmd_SerialConfigCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    int portIndex;
    GPS_OEMCmd_PublishCmd_Payload_t Payload;
} GPS_OEMCmd_PublishCmd_t;

/*
** OEM log handler command types.
*/

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerRegisterCmd_Payload_t Payload;
} GPS_OEMLog_HandlerRegisterCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerUnregisterCmd_Payload_t Payload;
} GPS_OEMLog_HandlerUnregisterCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_AddCallbackCmd_Payload_t Payload;
} GPS_OEMLog_AddCallbackCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_ClearCallbackCmd_Payload_t Payload;
} GPS_OEMLog_ClearCallbackCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_GetHandlerHkCmd_Payload_t Payload;
} GPS_OEMLog_GetHandlerHkCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_GetMsgStatCmd_Payload_t Payload;
} GPS_OEMLog_GetMsgStatCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_SetHandlerStatusCmd_Payload_t Payload;
} GPS_OEMLog_SetHandlerStatusCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_GetHandlerStatusCmd_Payload_t Payload;
} GPS_OEMLog_GetHandlerStatusCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerActivateCmd_Payload_t Payload;
} GPS_OEMLog_HandlerActivateCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerDeactivateCmd_Payload_t Payload;
} GPS_OEMLog_HandlerDeactivateCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerGoDormantCmd_Payload_t Payload;
} GPS_OEMLog_HandlerGoDormantCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerWakeupCmd_Payload_t Payload;
} GPS_OEMLog_HandlerWakeupCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerActivateAllCmd_Payload_t Payload;
} GPS_OEMLog_HandlerActivateAllCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerDeactivateAllCmd_Payload_t Payload;
} GPS_OEMLog_HandlerDeactivateAllCmd_t;

/*
** Miscellaneous log handler command types.
*/


typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_HandlerSetBrokenCmd_Payload_t Payload;
} GPS_OEMLog_HandlerSetBrokenCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_GetHandlerMsgLengthCmd_Payload_t Payload;
} GPS_OEMLog_GetHandlerMsgLengthCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_GetHandlerNameCmd_Payload_t Payload;
} GPS_OEMLog_GetHandlerNameCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_ResetHandlerCountersCmd_Payload_t Payload;
} GPS_OEMLog_ResetHandlerCountersCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_DumpRecentMsgCmd_Payload_t Payload;
} GPS_OEMLog_DumpRecentMsgCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_IgnoreChecksumCmd_Payload_t Payload;
} GPS_OEMLog_IgnoreChecksumCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_DoNotIgnoreChecksumCmd_Payload_t Payload;
} GPS_OEMLog_DonotIgnoreChecksumCmd_t;

/*
** "dangerous" driver command types.
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_LockHandlersCmd_Payload_t Payload;
} GPS_OEMLog_LockHandlersCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEMLog_UnlockHandlersCmd_Payload_t Payload;
} GPS_OEMLog_UnlockHandlersCmd_t;


/*************************************************************************/
/*
** Type definition (GPS housekeeping)
*/

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} GPS_ReportTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    GPS_HkTlm_Payload_t Payload;
} GPS_HkTlm_t;


#endif
      