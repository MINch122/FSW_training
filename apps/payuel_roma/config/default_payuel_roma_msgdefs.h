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
 *   message constant definitions.
 *
 *  For PAYUEL_ROMA this is only the function/command code definitions
 */
#ifndef PAYUEL_ROMA_MSGDEFS_H
#define PAYUEL_ROMA_MSGDEFS_H

#include "common_types.h"
#include "payuel_roma_fcncodes.h"

typedef struct PAYUEL_ROMA_DisplayParam_Payload
{
    uint32 ValU32;                            /**< 32 bit unsigned integer value */
    int16  ValI16;                            /**< 16 bit signed integer value */
    char   ValStr[PAYUEL_ROMA_STRING_VAL_LEN]; /**< An example string */
} PAYUEL_ROMA_DisplayParam_Payload_t;

/******************************************/
/*     Telecommand Table                  */
/******************************************/

/* LOG (PORT 16) */

typedef struct PAYUEL_ROMA_GetSpecificLine_Payload
{
    uint8 type;
    uint16 line_no;
} PAYUEL_ROMA_GetSpecificLine_Payload_t;

typedef struct PAYUEL_ROMA_GetMultipleLines_Payload
{
    uint8 type;
    uint16 line_start;
    uint16 line_stop;
} PAYUEL_ROMA_GetMultipleLines_Payload_t;

typedef struct PAYUEL_ROMA_GetLatestLine_Payload
{
    uint8 type;
} PAYUEL_ROMA_GetLatestLine_Payload_t;

typedef struct PAYUEL_ROMA_GetLatestNLines_Payload
{
    uint8 type;
    uint16 n_lines;
} PAYUEL_ROMA_GetLatestNLines_Payload_t;

typedef struct PAYUEL_ROMA_ClearAllLines_Payload
{
    uint8 type;
    uint16 code;
} PAYUEL_ROMA_ClearAllLines_Payload_t;


/* SCHEDULE (PORT 17) */

typedef struct PAYUEL_ROMA_GetSingleEntry_Payload
{
    uint8 type;
    uint16 entry_index;
} PAYUEL_ROMA_GetSingleEntry_Payload_t;

typedef struct PAYUEL_ROMA_GetMultipleEntries_Payload
{
    uint8 type;
    uint16 start_index;
    uint16 end_index;
} PAYUEL_ROMA_GetMultipleEntries_Payload_t;

typedef struct PAYUEL_ROMA_AddEntry_Payload
{
    uint8 type;
    uint64 time_ms;
    uint32 repeat_every_ms;
    int8 repeat_max;
    uint8 command[64];     // 예약 명령어 최대 길이 64
} PAYUEL_ROMA_AddEntry_Payload_t;

typedef struct PAYUEL_ROMA_RemoveEntry_Payload
{
    uint8 type;
    uint8 entry_index;
} PAYUEL_ROMA_RemoveEntry_Payload_t;

typedef struct PAYUEL_ROMA_GetUsedSlots_Payload
{
    uint8 type;
} PAYUEL_ROMA_GetUsedSlots_Payload_t;


/* ROUTING (PORT 18) */

typedef struct PAYUEL_ROMA_SetRouteDefault_Payload
{
    uint8 type;
} PAYUEL_ROMA_SetRouteDefault_Payload_t;

typedef struct PAYUEL_ROMA_ResetRoute_Payload
{
    uint8 type;
} PAYUEL_ROMA_ResetRoute_Payload_t;

typedef struct PAYUEL_ROMA_LoadRoute_Payload
{
    uint8 type;
} PAYUEL_ROMA_LoadRoute_Payload_t;

typedef struct PAYUEL_ROMA_SaveRoute_Payload
{
    uint8 type;
} PAYUEL_ROMA_SaveRoute_Payload_t;

typedef struct PAYUEL_ROMA_SendRoute_Payload
{
    uint8 type;
} PAYUEL_ROMA_SendRoute_Payload_t;

typedef struct PAYUEL_ROMA_SetRoute_Payload
{
    uint8 type;
    char route[127];      // route 문자열 최대 길이 127, 전체 128 byte
} PAYUEL_ROMA_SetRoute_Payload_t;


/* PARAMETERS (PORT 19) */

typedef struct PAYUEL_ROMA_ParGet_Payload
{
    uint8 type;
    uint8 table;
    uint8 param;
} PAYUEL_ROMA_ParGet_Payload_t;

typedef struct PAYUEL_ROMA_ParSet_Payload
{
    uint8 type;
    uint8 table;
    uint8 param;
    int32 value;
} PAYUEL_ROMA_ParSet_Payload_t;

typedef struct PAYUEL_ROMA_ParDefaults_Payload
{
    uint8 type;
    uint8 table;
} PAYUEL_ROMA_ParDefaults_Payload_t;

typedef struct PAYUEL_ROMA_ParSave_Payload
{
    uint8 type;
    uint8 table;
} PAYUEL_ROMA_ParSave_Payload_t;

typedef struct PAYUEL_ROMA_ParRestore_Payload
{
    uint8 type;
    uint8 table;
} PAYUEL_ROMA_ParRestore_Payload_t;

typedef struct PAYUEL_ROMA_ParLoad_Payload
{
    uint8 type;
    uint8 table;
} PAYUEL_ROMA_ParLoad_Payload_t;

typedef struct PAYUEL_ROMA_ParSetOob_Payload
{
    uint8 type;
    uint8 table;
    uint8 enable;
} PAYUEL_ROMA_ParSetOob_Payload_t;


/* REMOTE TERMINAL (PORT 20) */

typedef struct PAYUEL_ROMA_SendCommand_Payload
{
    char cmd[128];                 // NOTE: must be NULL terminated
} PAYUEL_ROMA_SendCommand_Payload_t;


/* PAYLOAD OPERATIONS (PORT 8) */

typedef struct PAYUEL_ROMA_SendMsg_Payload
{
    char msg[128];                 // NOTE: no need for null termination
} PAYUEL_ROMA_SendMsg_Payload_t;

/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
*/

typedef struct PAYUEL_ROMA_HkTlm_Payload
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 spare[2];
} PAYUEL_ROMA_HkTlm_Payload_t;

typedef struct PAYUEL_ROMA_BcnTlm_Payload
{
    uint8 random; // 추후 정할 예정
} PAYUEL_ROMA_BcnTlm_Payload_t;

#endif
