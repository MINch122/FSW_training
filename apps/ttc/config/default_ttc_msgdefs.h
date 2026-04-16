/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 *   Specification for the TTC command and telemetry
 *   message payload and constant definitions.
 */
#ifndef DEFAULT_TTC_MSGDEFS_H
#define DEFAULT_TTC_MSGDEFS_H

#include "common_types.h"
#include "ttc_fcncodes.h"
#include "ttc_internal_cfg.h"

typedef struct __attribute__((packed)) {
    uint32 ExecutionTimeAbsolute;
    uint16 StalenessThreshold;
    uint16 EntryId;
    uint16 GroupId;
    uint8  ExecutionType;
    uint16 CommandSize;
    uint8  Command[TTC_PLATFORM_MAX_COMMAND_SIZE];
} TTC_InsertAbsCmdEntryCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 ExecutionTimeRelative;
    uint16 StalenessThreshold;
    uint16 EntryId;
    uint16 GroupId;
    uint8  ExecutionType;
    uint16 CommandSize;
    uint8  Command[TTC_PLATFORM_MAX_COMMAND_SIZE];
} TTC_InsertRelCmdEntryCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 EntryId;
    uint16 GroupId;
} TTC_DeleteEntryCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 GroupId;
} TTC_DeleteGroupCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 GroupId;
    uint16 EntryId;
    bool PersistentExecution;
} TTC_ExecuteEntryCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 GroupId;
    bool PersistentExecution;
} TTC_ExecuteGroupCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 EntryId;
    uint16 GroupId;
} TTC_PlumbEntryInitCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 EntryId;
    uint16 GroupId;
    uint16 Offset;
    uint16 ChunkSize;
    uint8  Data[TTC_PLATFORM_MAX_CHUNK_SIZE];
} TTC_PlumbEntryWriteCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 EntryId;
    uint16 GroupId;
    uint32 TimeTag;
    uint16 StalenessThreshold;
    uint16 CmdSize;
    uint8  TimeTagType;
    uint8  ExecutionType;
} TTC_PlumbEntryFinalizeCmd_Payload_t;

typedef struct __attribute__((packed)) {
    uint16 EntryId;
    uint16 GroupId;
} TTC_PlumbDeleteReservedEntryCmd_Payload_t;

/*************************************************************************/
/*
** Type definition (Ttc housekeeping)
*/

typedef struct TTC_HkTlm_Payload
{
    uint8 CommandCounter;
    uint8 CommandErrorCounter;
    uint8 spare[2];
} TTC_HkTlm_Payload_t;

#endif
