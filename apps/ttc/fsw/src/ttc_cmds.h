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
 *   This file contains the prototypes for the Ttc Ground Command-handling functions
 */

#ifndef TTC_CMDS_H
#define TTC_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "ttc_msg.h"

void TTC_SendHkCmd(const TTC_SendHkCmd_t* Msg);
void TTC_ReportCmd(const TTC_ReportCmd_t* Msg);
void TTC_ResetCountersCmd(const TTC_ResetCountersCmd_t* Msg);
void TTC_NoopCmd(const TTC_NoopCmd_t* Msg);

void TTC_GetTimelineHkCmd(const TTC_GetTimelineHkCmd_t* Msg);
void TTC_ResetTimelineHkCmd(const TTC_ResetTimelineHkCmd_t* Msg);

void TTC_GetPendingEntryCountCmd(const TTC_GetPendingEntryCountCmd_t* Msg);
void TTC_GetNextEntryIdCmd(const TTC_GetNextEntryIdCmd_t* Msg);
void TTC_GetNextExecutionTimeCmd(const TTC_GetNextExecutionTimeCmd_t* Msg);

void TTC_InsertAbsCmdEntryCmd(const TTC_InsertAbsCmdEntryCmd_t* Msg);
void TTC_InsertRelCmdEntryCmd(const TTC_InsertRelCmdEntryCmd_t* Msg);

void TTC_DeleteEntryCmd(const TTC_DeleteEntryCmd_t* Msg);
void TTC_DeleteGroupCmd(const TTC_DeleteGroupCmd_t* Msg);
void TTC_DeleteAllEntriesCmd(const TTC_DeleteAllEntriesCmd_t* Msg);

void TTC_ExecuteEntryCmd(const TTC_ExecuteEntryCmd_t* Msg);
void TTC_ExecuteGroupCmd(const TTC_ExecuteGroupCmd_t* Msg);

void TTC_PauseTimelineProcessingCmd(const TTC_PauseTimelineProcessingCmd_t* Msg);
void TTC_ResumeTimelineProcessingCmd(const TTC_ResumeTimelineProcessingCmd_t* Msg);

void TTC_PlumbEntryInitCmd(const TTC_PlumbEntryInitCmd_t* Msg);
void TTC_PlumbEntryWriteCmd(const TTC_PlumbEntryWriteCmd_t* Msg);
void TTC_PlumbEntryFinalizeCmd(const TTC_PlumbEntryFinalizeCmd_t* Msg);
void TTC_PlumbPurgeTimelineCmd(const TTC_PlumbPurgeTimelineCmd_t* Msg);
void TTC_PlumbDeleteReservedEntryCmd(const TTC_PlumbDeleteReservedEntryCmd_t* Msg);

#endif /* TTC_CMDS_H */
