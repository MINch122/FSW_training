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
 *   Specification for the PAYUEL_ROMA command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in payuel_roma_msgdefs.h.
 */
#ifndef PAYUEL_ROMA_MSGSTRUCT_H
#define PAYUEL_ROMA_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "payuel_roma_mission_cfg.h"
#include "payuel_roma_msgdefs.h"
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
} PAYUEL_ROMA_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_ROMA_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
} PAYUEL_ROMA_DisplayParamCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t           CommandHeader;
} PAYUEL_ROMA_ClockSyncCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t            CommandHeader;
} PAYUEL_ROMA_CommTestCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
} PAYUEL_ROMA_LogTestCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
} PAYUEL_ROMA_TransTestCmd_t;


/************************************************************************
 * Telecommand
 ************************************************************************/

/* LOG (PORT 16) */

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;   /* Command header*/
    PAYUEL_ROMA_GetSpecificLine_Payload_t    Payload;
} PAYUEL_ROMA_GetSpecificLineCmd_t;    

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_GetMultipleLines_Payload_t   Payload;
} PAYUEL_ROMA_GetMultipleLinesCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_GetLatestLine_Payload_t      Payload;
} PAYUEL_ROMA_GetLatestLineCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_GetLatestNLines_Payload_t    Payload;
} PAYUEL_ROMA_GetLatestNLinesCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ClearAllLines_Payload_t      Payload;
} PAYUEL_ROMA_ClearAllLinesCmd_t;


/* SCHEDULE (PORT 17) */

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_GetSingleEntry_Payload_t     Payload;
} PAYUEL_ROMA_GetSingleEntryCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_GetMultipleEntries_Payload_t Payload;
} PAYUEL_ROMA_GetMultipleEntriesCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_AddEntry_Payload_t           Payload;
} PAYUEL_ROMA_AddEntryCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_RemoveEntry_Payload_t        Payload;
} PAYUEL_ROMA_RemoveEntryCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_GetUsedSlots_Payload_t       Payload;
} PAYUEL_ROMA_GetUsedSlotsCmd_t;


/* ROUTING (PORT 18) */

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_SetRouteDefault_Payload_t    Payload;
} PAYUEL_ROMA_SetRouteDefaultCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ResetRoute_Payload_t         Payload;
} PAYUEL_ROMA_ResetRouteCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_LoadRoute_Payload_t          Payload;
} PAYUEL_ROMA_LoadRouteCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_SaveRoute_Payload_t          Payload;
} PAYUEL_ROMA_SaveRouteCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_SendRoute_Payload_t          Payload;
} PAYUEL_ROMA_SendRouteCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_SetRoute_Payload_t           Payload;
} PAYUEL_ROMA_SetRouteCmd_t;


/* PARAMETERS (PORT 19) */

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParGet_Payload_t             Payload;
} PAYUEL_ROMA_ParGetCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParSet_Payload_t             Payload;
} PAYUEL_ROMA_ParSetCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParDefaults_Payload_t        Payload;
} PAYUEL_ROMA_ParDefaultsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParSave_Payload_t            Payload;
} PAYUEL_ROMA_ParSaveCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParRestore_Payload_t         Payload;
} PAYUEL_ROMA_ParRestoreCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParLoad_Payload_t            Payload;
} PAYUEL_ROMA_ParLoadCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_ParSetOob_Payload_t          Payload;
} PAYUEL_ROMA_ParSetOobCmd_t;


/* REMOTE TERMINAL (PORT 20) */

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_SendCommand_Payload_t        Payload;
} PAYUEL_ROMA_SendCommandCmd_t;


/* PAYLOAD OPERATIONS (PORT 8) */

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
    PAYUEL_ROMA_SendMsg_Payload_t        Payload;
} PAYUEL_ROMA_SendMsgCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
} PAYUEL_ROMA_SyncRxCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
} PAYUEL_ROMA_SyncTxCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t            CommandHeader;
} PAYUEL_ROMA_PayInitCmd_t;


/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_ROMA_SendHkCmd_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    PAYUEL_ROMA_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} PAYUEL_ROMA_HkTlm_t;

/* Report SB MSG */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} PAYUEL_ROMA_ReportTlm_t;

#endif /* PAYUEL_ROMA_MSGSTRUCT_H */
