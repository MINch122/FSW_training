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
    GPS_OEM_Cmd_Log_Payload_t Payload;
} GPS_OEM_Cmd_Log_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_LogOnce_Payload_t Payload;
} GPS_OEM_Cmd_LogOnce_t;

#define a sizeof(GPS_OEM_Cmd_Log_t)

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_LogOnTime_Payload_t Payload;
} GPS_OEM_Cmd_LogOnTime_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_LogOnChanged_Payload_t Payload;
} GPS_OEM_Cmd_LogOnChanged_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_LogOnNew_Payload_t Payload;
} GPS_OEM_Cmd_LogOnNew_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_Unlog_Payload_t Payload;
} GPS_OEM_Cmd_Unlog_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_UnlogAll_Payload_t Payload;
} GPS_OEM_Cmd_UnlogAll_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_ElevationCutoff_Payload_t Payload;
} GPS_OEM_Cmd_ElevationCutoff_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_InterfaceMode_Payload_t Payload;
} GPS_OEM_Cmd_InterfaceMode_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_SerialConfig_Payload_t Payload;
} GPS_OEM_Cmd_SerialConfig_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Cmd_Publish_Payload_t Payload;
} GPS_OEM_Cmd_Publish_t;

/*
** OEM log handler command types.
*/

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerRegister_Payload_t Payload;
} GPS_OEM_Log_HandlerRegister_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerUnregister_Payload_t Payload;
} GPS_OEM_Log_HandlerUnregister_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_AddCallback_Payload_t Payload;
} GPS_OEM_Log_AddCallback_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_ClearCallbacks_Payload_t Payload;
} GPS_OEM_Log_ClearCallbacks_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_GetHandlerHk_Payload_t Payload;
} GPS_OEM_Log_GetHandlerHk_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_GetStat_Payload_t Payload;
} GPS_OEM_Log_GetStat_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerSetStatus_Payload_t Payload;
} GPS_OEM_Log_HandlerSetStatus_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerGetStatus_Payload_t Payload;
} GPS_OEM_Log_HandlerGetStatus_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerActivate_Payload_t Payload;
} GPS_OEM_Log_HandlerActivate_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerDeactivate_Payload_t Payload;
} GPS_OEM_Log_HandlerDeactivate_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerGoDormant_Payload_t Payload;
} GPS_OEM_Log_HandlerGoDormant_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerWakeup_Payload_t Payload;
} GPS_OEM_Log_HandlerWakeup_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerActivateAll_Payload_t Payload;
} GPS_OEM_Log_HandlerActivateAll_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerDeactivateAll_Payload_t Payload;
} GPS_OEM_Log_HandlerDeactivateAll_t;

/*
** Miscellaneous log handler command types.
*/


typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_HandlerMarkBroken_Payload_t Payload;
} GPS_OEM_Log_HandlerMarkBroken_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_GetMessageLength_Payload_t Payload;
} GPS_OEM_Log_GetMessageLength_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_GetHandlerName_Payload_t Payload;
} GPS_OEM_Log_GetHandlerName_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_ResetStat_Payload_t Payload;
} GPS_OEM_Log_ResetStat_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_GetRecentMessage_Payload_t Payload;
} GPS_OEM_Log_GetRecentMessage_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_RejectMissingCrc_Payload_t Payload;
} GPS_OEM_Log_RejectMissingCrc_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_IgnoreMissingCrc_Payload_t Payload;
} GPS_OEM_Log_IgnoreMissingCrc_t;

/*
** "dangerous" driver command types.
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_LockHandlers_Payload_t Payload;
} GPS_OEM_Log_LockHandlers_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    GPS_OEM_Log_UnlockHandlers_Payload_t Payload;
} GPS_OEM_Log_UnlockHandlers_t;


/*************************************************************************/
/*
** Type definition (GPS housekeeping)
*/

/* Report telemetry forwarded to the RPT app. The payload MUST be RPT_Report_t
 * so that sizeof(GPS_ReportTlm_t) == sizeof(RPT_ReportTlm_t); RPT drops any
 * subscribed message whose size differs (see RPT_VerifyReportLength). GPS still
 * caps the data it copies into ReturnValue at GPS_MISSION_REPORT_DATA_SIZE. */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} GPS_ReportTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    GPS_HkTlm_Payload_t Payload;
} GPS_HkTlm_t;


#endif
      