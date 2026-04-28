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
 *   This file contains the prototypes for the UELYSYS Payload Roma-SP Ground Command-handling functions
 */

#ifndef PAYUEL_ROMA_CMDS_H
#define PAYUEL_ROMA_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "payuel_roma_msg.h"

CFE_Status_t PAYUEL_ROMA_SendHkCmd(const PAYUEL_ROMA_SendHkCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_SendBcnCmd(const PAYUEL_ROMA_SendBcnCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_ResetCountersCmd(const PAYUEL_ROMA_ResetCountersCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_NoopCmd(const PAYUEL_ROMA_NoopCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_CommTestCmd(const PAYUEL_ROMA_CommTestCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ClockSyncCmd(const PAYUEL_ROMA_ClockSyncCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_LogTestCmd(const PAYUEL_ROMA_LogTestCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_TransTestCmd(const PAYUEL_ROMA_TransTestCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_GetSpecificLineCmd(const PAYUEL_ROMA_GetSpecificLineCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_GetMultipleLinesCmd(const PAYUEL_ROMA_GetMultipleLinesCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_GetLatestLineCmd(const PAYUEL_ROMA_GetLatestLineCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_GetLatestNLinesCmd(const PAYUEL_ROMA_GetLatestNLinesCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ClearAllLinesCmd(const PAYUEL_ROMA_ClearAllLinesCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_GetSingleEntryCmd(const PAYUEL_ROMA_GetSingleEntryCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_GetMultipleEntriesCmd(const PAYUEL_ROMA_GetMultipleEntriesCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_AddEntryCmd(const PAYUEL_ROMA_AddEntryCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_RemoveEntryCmd(const PAYUEL_ROMA_RemoveEntryCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_GetUsedSlotsCmd(const PAYUEL_ROMA_GetUsedSlotsCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_SetRouteDefaultCmd(const PAYUEL_ROMA_SetRouteDefaultCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ResetRouteCmd(const PAYUEL_ROMA_ResetRouteCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_LoadRouteCmd(const PAYUEL_ROMA_LoadRouteCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_SaveRouteCmd(const PAYUEL_ROMA_SaveRouteCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_SendRouteCmd(const PAYUEL_ROMA_SendRouteCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_SetRouteCmd(const PAYUEL_ROMA_SetRouteCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_ParGetCmd(const PAYUEL_ROMA_ParGetCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ParSetCmd(const PAYUEL_ROMA_ParSetCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ParDefaultsCmd(const PAYUEL_ROMA_ParDefaultsCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ParSaveCmd(const PAYUEL_ROMA_ParSaveCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ParRestoreCmd(const PAYUEL_ROMA_ParRestoreCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ParLoadCmd(const PAYUEL_ROMA_ParLoadCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_ParSetOobCmd(const PAYUEL_ROMA_ParSetOobCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_SendCommandCmd(const PAYUEL_ROMA_SendCommandCmd_t *Msg);

CFE_Status_t PAYUEL_ROMA_SendMsgCmd(const PAYUEL_ROMA_SendMsgCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_SyncRxCmd(const PAYUEL_ROMA_SyncRxCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_SyncTxCmd(const PAYUEL_ROMA_SyncTxCmd_t *Msg);
CFE_Status_t PAYUEL_ROMA_PayInitCmd(const PAYUEL_ROMA_PayInitCmd_t *Msg);

#endif /* PAYUEL_ROMA_CMDS_H */
