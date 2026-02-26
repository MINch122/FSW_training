/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
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
 *   Prototypes for the EPS Application Ground Command-handling functions
 */

#ifndef EPS_CMDS_H
#define EPS_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "eps_msg.h"

/**
 * @brief Send report with command result
 */
void EPS_SendReport(const void* cmd,
                    const void* data,
                    uint16 dataSize,
                    int32 retCode,
                    uint8 retType);

/*
** Basic Commands
*/
CFE_Status_t EPS_SendHkCmd(const EPS_SendHkCmd_t *Msg);
CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg);
CFE_Status_t EPS_ResetCountersCmd(const EPS_ResetCountersCmd_t *Msg);
CFE_Status_t EPS_NoopCmd(const EPS_NoopCmd_t *Msg);

/*
** Power Interface Commands
*/
CFE_Status_t EPS_Power_If_Get_Cmd(const EPS_Power_If_Get_Cmd_t *Msg);
CFE_Status_t EPS_Power_If_Set_Cmd(const EPS_Power_If_Set_Cmd_t *Msg);
CFE_Status_t EPS_Power_If_List_Cmd(const EPS_Power_If_List_Cmd_t *Msg);

/*
** Housekeeping & Watchdog Commands
*/
CFE_Status_t EPS_Get_Hk_Cmd(const EPS_Get_Hk_Cmd_t *Msg);
CFE_Status_t EPS_Gnd_Watchdog_Clear_Cmd(const EPS_Gnd_Watchdog_Clear_Cmd_t *Msg);

/*
** Remote Parameter Commands (추가됨)
*/
CFE_Status_t EPS_Param_Set_Cmd(const EPS_Param_Set_Cmd_t *Msg);
CFE_Status_t EPS_Param_Get_Cmd(const EPS_Param_Get_Cmd_t *Msg);
CFE_Status_t EPS_Get_Full_Table_Cmd(const EPS_Get_Full_Table_Cmd_t *Msg);

/*
** Table Save/Load Commands (추가됨)
*/

CFE_Status_t EPS_Table_Save_Cmd(const EPS_Table_Save_Cmd_t *Msg);
CFE_Status_t EPS_Table_Load_Cmd(const EPS_Table_Load_Cmd_t *Msg);

#endif /* EPS_CMDS_H */