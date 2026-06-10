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

/*
** Common command handlers.
*/
/*
 * NOOP: health check for the EPS app. No payload fields; increments the EPS
 * command counter and emits the app version event.
 */
CFE_Status_t EPS_NoopCmd(const EPS_NoopCmd_t *Msg);
/*
 * ResetCounters: clears EPS command/error/HK/beacon error counters. No payload
 * fields.
 */
CFE_Status_t EPS_ResetCountersCmd(const EPS_ResetCountersCmd_t *Msg);
/* CFE_Status_t EPS_SendHkCmd(const EPS_SendHkCmd_t *Msg); */ /* Disabled: EPS_SendHkCmd is not implemented */
/*
 * SendBcn: refreshes EPS beacon data from hardware and transmits the beacon
 * telemetry packet on the software bus. No payload fields.
 *
 * Beacon byte meanings:
 * - PMU.sm_en_mask: bit0..7 = PMU submodule enable status 0..7, 1=enabled.
 * - PDU.out_i/out_en: channels 8,10,12,14,15,16,18,19,20,21,22,23.
 */
CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg);
/*
 * ReportBcn: refreshes beacon data and prints a human-readable beacon report.
 * Uses the same packed byte meanings documented on EPS_SendBcnCmd.
 */
CFE_Status_t EPS_ReportBcnCmd(const EPS_ReportBcnCmd_t *Msg);

/*
** P80 power interface command handlers.
*/
/*
 * P80 Power_If_Get: read one P80 named power interface channel.
 */
CFE_Status_t EPS_P80_Power_If_Get_Cmd(const EPS_P80_Power_If_Get_Cmd_t *Msg);
/*
 * P80 Power_If_Set: set one P80 named power interface channel.
 */
CFE_Status_t EPS_P80_Power_If_Set_Cmd(const EPS_P80_Power_If_Set_Cmd_t *Msg);
/*
 * P80 Power_If_List: list available named power interface channels.
 */
CFE_Status_t EPS_P80_Power_If_List_Cmd(const EPS_P80_Power_If_List_Cmd_t *Msg);

/*
** Housekeeping and P80 watchdog command handlers.
*/
/*
 * Get_HK: read and print housekeeping for one EPS node.
 */
CFE_Status_t EPS_Get_HK_Cmd(const EPS_Get_HK_Cmd_t *Msg);
/*
 * Get_HK_All: read and print housekeeping for PMU, PDU, ACU1, ACU2, and BP8.
 * No payload fields.
 */
CFE_Status_t EPS_Get_HK_All_Cmd(const EPS_Get_HK_All_Cmd_t *Msg);
/*
 * P80_Gnd_Watchdog_Clear: clear the ground watchdog on one P80 node.
 */
CFE_Status_t EPS_P80_Gnd_Watchdog_Clear_Cmd(const EPS_P80_Gnd_Watchdog_Clear_Cmd_t *Msg);
/*
 * P80_Gnd_Watchdog_Clear_All: clear ground watchdogs on all P80 nodes.
 */
CFE_Status_t EPS_P80_Gnd_Watchdog_Clear_All_Cmd(const EPS_P80_Gnd_Watchdog_Clear_All_Cmd_t *Msg);

/*
** Remote parameter command handlers.
*/
/*
 * RParam_Set: write one remote parameter by table address.
 */
CFE_Status_t EPS_RParam_Set_Cmd(const EPS_RParam_Set_Cmd_t *Msg);
/*
 * RParam_Get: read one remote parameter by table address and print raw bytes.
 */
CFE_Status_t EPS_RParam_Get_Cmd(const EPS_RParam_Get_Cmd_t *Msg);
/*
 * RParam_Get_Full_Table: download table specification and values, then print
 * each row with decoded type/value.
 */
CFE_Status_t EPS_RParam_Get_Full_Table_Cmd(const EPS_RParam_Get_Full_Table_Cmd_t *Msg);
/*
 * RParam_Save_All: persist all parameter tables on a node.
 */
CFE_Status_t EPS_RParam_Save_All_Cmd(const EPS_RParam_Save_All_Cmd_t *Msg);
/*
 * RParam_Save_To_Store: save one remote table to a named param-4 store.
 */
CFE_Status_t EPS_RParam_Save_To_Store_Cmd(const EPS_RParam_Save_To_Store_Cmd_t *Msg);
/*
 * RParam_Load_From_Store: load one remote table from a named param-4 store.
 */
CFE_Status_t EPS_RParam_Load_From_Store_Cmd(const EPS_RParam_Load_From_Store_Cmd_t *Msg);

/*
** Remote parameter table command handlers.
*/
/*
 * RParam_Table_Save: persist one remote table to its primary/default store.
 */
CFE_Status_t EPS_RParam_Table_Save_Cmd(const EPS_RParam_Table_Save_Cmd_t *Msg);
/*
 * RParam_Table_Load: reload one remote table from its primary/default store.
 */
CFE_Status_t EPS_RParam_Table_Load_Cmd(const EPS_RParam_Table_Load_Cmd_t *Msg);

/*
** CSP standard service command handlers.
*/
/*
 * CSP_PS: request and print the remote task/process list.
 */
CFE_Status_t EPS_CSP_PS_Cmd(const EPS_CSP_PS_Cmd_t *Msg);
/*
 * CSP_MemFree: query free memory on one EPS node.
 */
CFE_Status_t EPS_CSP_MemFree_Cmd(const EPS_CSP_MemFree_Cmd_t *Msg);
/*
 * CSP_BufFree: query free CSP buffer count on one EPS node.
 */
CFE_Status_t EPS_CSP_BufFree_Cmd(const EPS_CSP_BufFree_Cmd_t *Msg);
/*
 * CSP_Uptime: query uptime on one EPS node.
 */
CFE_Status_t EPS_CSP_Uptime_Cmd(const EPS_CSP_Uptime_Cmd_t *Msg);
/*
 * CSP_Ping: send a CSP ping to one EPS node.
 */
CFE_Status_t EPS_CSP_Ping_Cmd(const EPS_CSP_Ping_Cmd_t *Msg);
/*
 * CSP_Reboot: send a CSP reboot request to one EPS node.
 */
CFE_Status_t EPS_CSP_Reboot_Cmd(const EPS_CSP_Reboot_Cmd_t *Msg);

#endif /* EPS_CMDS_H */
