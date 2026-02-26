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
#ifndef DEFAULT_EPS_MSGSTRUCT_H
#define DEFAULT_EPS_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "default_eps_mission_cfg.h"
#include "default_eps_msgdefs.h"
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





typedef struct __attribute__((packed)){
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Power_If_Get_Cmd_Payload_t Payload;
}EPS_Power_If_Get_Cmd_t;

typedef struct __attribute__((packed)){
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Power_If_Set_Cmd_Payload_t Payload;
}EPS_Power_If_Set_Cmd_t;

typedef struct __attribute__((packed)){
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Power_If_List_Cmd_Payload_t Payload;
}EPS_Power_If_List_Cmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Get_Hk_Cmd_Payload_t Payload;   
}EPS_Get_Hk_Cmd_t;


typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Gnd_Watchdog_Clear_Cmd_Payload_t Payload;
}EPS_Gnd_Watchdog_Clear_Cmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Param_Set_Cmd_Payload_t Payload;
}EPS_Param_Set_Cmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Param_Get_Cmd_Payload_t Payload;
}EPS_Param_Get_Cmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Get_Full_Table_Cmd_Payload_t Payload;
}EPS_Get_Full_Table_Cmd_t;


typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Table_Save_Cmd_Payload_t Payload;
}EPS_Table_Save_Cmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
    EPS_Table_Save_Cmd_Payload_t Payload;
}EPS_Table_Load_Cmd_t;

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

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    EPS_BcnTlm_P80_Payload_t Payload;
} EPS_BcnTlm_P80_t;

#endif
      