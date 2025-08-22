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


#ifndef ADCS_MSGSTRUCT_H
#define ADCS_MSGSTRUCT_H

#include "adcs_msgdefs.h"

#include "cfe_msg_hdr.h"

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
} ADCS_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCS_ResetCountersCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GpioEnHighCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GpioEnLowCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GpioBootHighCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GpioBootLowCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_ExitBootLoaderCmd_t;

typedef struct{
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_ResetCmd_t;

/*************************************************************************/
/*
** Type definition (Housekeeping)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCS_SendHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCS_SendBcnCmd_t;


/********************************************************
 * 
 * COSMIC Actual Set Command structure
 * Upper functions are just the references
 * 
 ********************************************************/
typedef struct{ // ID 2
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS_CurrentUnixTimeCmd_Payload_t Payload;
} ADCS_CurrentUnixTimeCmd_t;

typedef struct{ // ID 42
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS_ControlEstimationModeCmd_Payload_t Payload;
} ADCS_ControlEstimationModeCmd_t;

typedef struct{ // ID 48
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS_ReferenceLLHTargetCmd_Payload_t Payload;
} ADCS_ReferenceLLHTargetCmd_t;

typedef struct{ // ID 51
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS_OrbitModeCmd_Payload_t Payload;
} ADCS_OrbitModeCmd_t;

typedef struct{ // ID 54
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS_ReferenceRPYvaluesCmd_Payload_t Payload;
} ADCS_ReferenceRPYvaluesCmd_t;

typedef struct{ // ID 68
    CFE_MSG_CommandHeader_t CommandHeader;
    ADCS_SatOrbitParamConfigCmd_Payload_t Payload;
} ADCS_SatOrbitParamConfigCmd_t;

/********************************************************
 * 
 * COSMIC Actual Get Command structure
 * Everything is No arguments
 * 
 ********************************************************/
typedef struct{ // ID 133
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetCurrentUnixTimeCmd_t;

typedef struct{ // ID 150
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetControlEstimationModeCmd_t;


typedef struct{ // ID 157
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetReferenceLLHTargetCmd_t;

typedef struct{ // ID 162
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetOrbitModeCmd_t;

typedef struct{  // ID 170
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetRawCubeSenseSunCmd_t;

typedef struct{ // ID 183
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetPowerStateCmd_t;

typedef struct{ // ID 196
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetSatOrbitParamConfigCmd_t;

typedef struct{ // ID 203
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetRawCSSSensorCmd_t;

typedef struct{ // ID 204
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetRawGYRSensorCmd_t;

typedef struct{ // ID 207
    CFE_MSG_CommandHeader_t CommandHeader;
} ADCS_GetCalibratedGYRSensorCmd_t;


/********************************************************
 * 
 * ADCS Telemetry Msg structure
 * 
 ********************************************************/
/* Beacon SB MSG */
typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    ADCS_BcnTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} ADCS_BcnTlm_t;

/* Housekeeping SB MSG */
typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    ADCS_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} ADCS_HkTlm_t;

#endif /* _adcs_app_msg_h_ */