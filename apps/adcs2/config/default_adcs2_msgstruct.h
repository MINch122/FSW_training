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
 *   Specification for the ADCS command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in ci_lab_msgdefs.h.
 */


#ifndef ADCS2_MSGSTRUCT_H
#define ADCS2_MSGSTRUCT_H

#include "adcs2_msgdefs.h"

#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCS2_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCS2_ResetCountersCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_ResetCmd_t;

/********************************************************
 * 
 * BEE1006 Actual Set Command structure
 * Upper functions are just the references
 * 
 ********************************************************/
typedef struct{ // ID 2
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_CurrentUnixTimeCmd_Payload_t Payload;
} ADCS2_CurrentUnixTimeCmd_t;

typedef struct { // ID 7
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_PersistConfigCmd_t;

typedef struct { // ID 42
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS2_ControlEstimationMode_Cmn_Payload_t Payload;
} ADCS2_ControlEstimationModeCmd_t;

typedef struct { // ID 56
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS2_PowerState_Cmn_Payload_t Payload;
} ADCS2_PowerStateCmd_t;

typedef struct { // ID 65
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS2_MountingConfig_Cmn_Payload_t Payload;
} ADCS2_MountingConfigCmd_t;

/********************************************************
 * 
 * BEE1006 Actual Get Command structure
 * Everything is No arguments
 * 
 ********************************************************/
typedef struct{ // ID 133
	CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_GetCurrentUnixTimeCmd_t;

typedef struct{ // ID 150
	CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_GetControlEstimationModeCmd_t;

typedef struct{ // ID 180
	CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_GetRawMAGSensorCmd_t;

typedef struct{ // ID 183
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_GetPowerStateCmd_t;

typedef struct{ // ID 193
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_GetMountingConfigCmd_t;

typedef struct{ // ID 204
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS2_GetRawGYRSensorCmd_t;



/********************************************************
 * 
 * BEE1006 Actual Commissioing Command structure
 * Everything is No arguments
 * 
 ********************************************************/
typedef struct{ // COMM 01
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm01Cmd_t;

typedef struct{ // COMM 02
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm02Cmd_t;

typedef struct{ // COMM 03
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm03Cmd_t;

typedef struct{ // COMM 04
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm04Cmd_t;

typedef struct{ // COMM 05
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm05Cmd_t;

typedef struct{ // COMM 06
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm06Cmd_t;

typedef struct{ // COMM 07
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm07Cmd_t;

typedef struct{ // COMM 08
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm08Cmd_t;

typedef struct{ // COMM 09
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm09Cmd_t;

typedef struct{ // COMM 10
	CFE_MSG_CommandHeader_t CommandHeader;
	ADCS2_COMM_FLAG_Payload_t	Payload;
} ADCS2_Comm10Cmd_t;



/********************************************************
 * 
 * ADCS Telemetry Msg structure
 * 
 ********************************************************/
/* Beacon SB MSG */
// typedef struct
// {
//     CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
//     ADCS2_BcnTlm_Payload_t Payload;         /**< \brief Telemetry payload */
//     bool IsSunlight;
// } ADCS2_BcnTlm_t;

/* Housekeeping SB MSG */
// typedef struct
// {
//     CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
//     ADCS2_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
// } ADCS2_HkTlm_t;

/* Report SB MSG */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} ADCS2_ReportTlm_t;


#endif /* _adcs2_app_msg_h_ */