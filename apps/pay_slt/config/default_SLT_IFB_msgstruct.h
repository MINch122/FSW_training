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
 *   Specification for the SAMPLE_APP command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in sample_app_msgdefs.h.
 */
#ifndef SLT_IFB_MSGSTRUCT_H
#define SLT_IFB_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "SLT_IFB_mission_cfg.h"
#include "SLT_IFB_msgdefs.h"
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
typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SLT_IFB_NoopCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SLT_IFB_ResetCountersCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SLT_IFB_TransactionCmd_t;

typedef struct  __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SLT_IFB_RPARAM_GetCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SLT_IFB_SaveCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SLT_IFB_HkTlm_Payload_t Payload;
} SLT_IFB_SendHkCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SLT_IFB_RPT_t;



#endif
