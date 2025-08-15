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
 *   Specification for the STRX_APP command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in strx_app_msgdefs.h.
 */
#ifndef STRX_APP_MSGSTRUCT_H
#define STRX_APP_MSGSTRUCT_H
#define MAX_TM_DATASIZE 170
#define MAX_RAW_DATASIZE 200
#define MAX_TX_DATASIZE 170

/************************************************************************
 * Includes
 ************************************************************************/

#include "strx_app_mission_cfg.h"
#include "strx_app_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

/*************************************************************************/

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
} STRX_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} STRX_APP_ResetCountersCmd_t;


/*************************************************************************/
/*
** Type definition (Strx App housekeeping)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} STRX_APP_SendHkCmd_t;

// typedef struct
// {
//     CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
//     STRX_APP_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
// } STRX_APP_HkTlm_t;

//STRX TAEHWAN DEF*************************************************************//
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;

} STRX_APP_NoArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 arg;

} STRX_U8ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    int8_t arg;

} STRX_8ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16 arg;

} STRX_U16ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint32 arg;

} STRX_U32ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint64 arg;

} STRX_U64ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    /* data */
} STRX_APP_ResetAppCmdCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    /* data */
} STRX_APP_ResetDeviceCmdCountersCmd_t;

/**************Telemetry******************** */

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    union {
        STRX_HkTlm_Payload_t hk;
        STRX_BcnTlm_Payload_t bcn;
    } Payload;
} STRX_Tlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} STRX_ReportTlm_t;

/* Size of each Tlm */
#define STRX_TLM_HK_SIZE  (offsetof(STRX_Tlm_t, Payload) + sizeof(STRX_HkTlm_Payload_t))
#define STRX_TLM_BCN_SIZE (offsetof(STRX_Tlm_t, Payload) + sizeof(STRX_BcnTlm_Payload_t))

#endif /* STRX_APP_MSGSTRUCT_H */
