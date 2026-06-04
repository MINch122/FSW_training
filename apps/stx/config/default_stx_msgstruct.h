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
 *   Specification for the STX command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in stx_msgdefs.h.
 */
#ifndef STX_MSGSTRUCT_H
#define STX_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "stx_mission_cfg.h"
#include "stx_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

/*************************************************************************/

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*  gs -> obc
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} STX_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} STX_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} STX_ParamInitCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    STX_Set3_Payload_t Payload;
} STX_ModuleIdInitCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_DisplayParam_Payload_t Payload;
} STX_DisplayParamCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
} STX_Set_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_SYMBOLRAtE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_TRANSMITPW_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set2_Payload_t Payload;
} STX_Set_CENTERFREQ_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_MODCOD_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_ROLLOFF_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_PILOTSIG_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_FECFRAME_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set3_Payload_t Payload;
} STX_Set_PRETX_DELAY_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set4_Payload_t Payload;
} STX_Set_ALLPRAM_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set1_Payload_t Payload;
} STX_Set_RS485_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_Set5_Payload_t Payload;
} STX_Set_MODULATION_INTERFACE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_DELFILE_Payload_t Payload;
} STX_DELFILE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_CREATEFILE_Payload_t Payload;
} STX_CREATEFILE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_TLM_WRITEFILE_Payload_t          Payload;
} STX_WRITEFILE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_OPENFILE_Payload_t Payload;
} STX_OPENFILE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
} STX_READFILE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    STX_SENDFILE_Payload_t Payload;
} STX_SENDFILE_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
} STX_Get_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} STX_SendHkCmd_t;
/*************************************************************************/
/*
** Type definition
*   obc <-> app
*
*/

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    STX_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} STX_HkTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    STX_BCNTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} STX_BCNTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    STX_SET_Tlm_Payload_t Payload;         /**< \brief Telemetry payload */
} STX_SetTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    STX_GET_Tlm_Payload_t Payload;         /**< \brief Telemetry payload */
} STX_GetTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    STX_FILE_Tlm_Payload_t Payload;         /**< \brief Telemetry payload */
} STX_FileTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} STX_ReportTlm_t;

#endif /* STX_MSGSTRUCT_H */
