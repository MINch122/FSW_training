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
 *   Specification for the EPS command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in cosmos_eps_msgdefs.h.
 */
#ifndef EPS_MSGSTRUCT_H
#define EPS_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "eps_mission_cfg.h"
#include "eps_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

/*************************************************************************/


/**
 * Noarg cmd template.
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} EPS_NoArgCmd_t;

typedef EPS_NoArgCmd_t  EPS_NoopCmd_t;
typedef EPS_NoArgCmd_t  EPS_ResetCountersCmd_t;
typedef EPS_NoArgCmd_t  EPS_ReportAppDataCmd_t;

typedef EPS_NoArgCmd_t  EPS_P31U_PingCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_ResetCountersCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_ResetWdtCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_HardResetCmd_t;

typedef EPS_NoArgCmd_t  EPS_P31U_GetHkAllCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_GetHkOutCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_GetHkViCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_GetHkWdtCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_GetHkBasicCmd_t;
typedef EPS_NoArgCmd_t  EPS_P31U_GetHkOldCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_GetHk_Payload_t Payload;
} EPS_P31U_GetHkCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetOutputSingle_Payload_t Payload;
} EPS_P31U_SetOutputSingleCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetOutputs_Payload_t Payload;
} EPS_P31U_SetOutputsCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetPvVolt_Payload_t Payload;
} EPS_P31U_SetPvVoltCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetPvAuto_Payload_t Payload;
} EPS_P31U_SetPvAutoCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetHeater_Payload_t Payload;
} EPS_P31U_SetHeaterCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_Config_Payload_t Payload;
} EPS_P31U_ConfigCmd_t;

typedef EPS_NoArgCmd_t EPS_P31U_GetConfigCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetConfig_Payload_t Payload;
} EPS_P31U_SetConfigCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_Config2_Payload_t Payload;
} EPS_P31U_Config2Cmd_t;

typedef EPS_NoArgCmd_t EPS_P31U_GetConfig2Cmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetConfig2_Payload_t Payload;
} EPS_P31U_SetConfig2Cmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_SetConfig3_Payload_t Payload;
} EPS_P31U_SetConfig3Cmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_P31U_Transaction_Payload_t Payload;
} EPS_P31U_TransactionCmd_t;


/*************************************************************************/
/*
** Type definition (EPS housekeeping)
*/
typedef EPS_NoArgCmd_t  EPS_SendHkCmd_t;
typedef EPS_NoArgCmd_t  EPS_SendBcnCmd_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} EPS_ReportTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    EPS_HkTlm_Payload_t Payload;
} EPS_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    EPS_BcnTlm_Payload_t Payload;
} EPS_BcnTlm_t;

#endif
      