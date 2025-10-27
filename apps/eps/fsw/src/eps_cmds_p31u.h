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
 *   Prototypes for the EPS Application Ground Command-handling functions
 */

#ifndef EPS_CMDS_P31U_H
#define EPS_CMDS_P31U_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "eps_msg.h"


void EPS_P31U_SetOutputSingleCmd(const EPS_P31U_SetOutputSingleCmd_t *Msg);
void EPS_P31U_SetOutputSingleInternalCmd(const EPS_P31U_SetOutputSingleCmd_t *Msg);
void EPS_P31U_SetOutputsCmd(const EPS_P31U_SetOutputsCmd_t *Msg);
void EPS_P31U_ResetWdtCmd(const EPS_P31U_ResetWdtCmd_t *Msg);
void EPS_P31U_ResetCountersCmd(const EPS_P31U_ResetCountersCmd_t *Msg);
void EPS_P31U_HardResetCmd(const EPS_P31U_HardResetCmd_t *Msg);

void EPS_P31U_GetHkAllCmd(const EPS_P31U_GetHkAllCmd_t *Msg);
void EPS_P31U_GetHkOutCmd(const EPS_P31U_GetHkOutCmd_t *Msg);
void EPS_P31U_GetHkOutInternalCmd(const EPS_P31U_GetHkOutCmd_t *Msg);
void EPS_P31U_GetHkViCmd(const EPS_P31U_GetHkViCmd_t *Msg);
void EPS_P31U_GetHkViInternalCmd(const EPS_P31U_GetHkViCmd_t *Msg);
void EPS_P31U_GetHkWdtCmd(const EPS_P31U_GetHkWdtCmd_t *Msg);
void EPS_P31U_GetHkBasicCmd(const EPS_P31U_GetHkBasicCmd_t *Msg);
void EPS_P31U_GetHkOldCmd(const EPS_P31U_GetHkOldCmd_t *Msg);
void EPS_P31U_GetHkCmd(const EPS_P31U_GetHkCmd_t *Msg);

void EPS_P31U_SetPvVoltCmd(const EPS_P31U_SetPvVoltCmd_t *Msg);
void EPS_P31U_SetPvAutoCmd(const EPS_P31U_SetPvAutoCmd_t *Msg);
void EPS_P31U_SetHeaterCmd(const EPS_P31U_SetHeaterCmd_t *Msg);
void EPS_P31U_GetConfigCmd(const EPS_P31U_GetConfigCmd_t *Msg);
void EPS_P31U_SetConfigCmd(const EPS_P31U_SetConfigCmd_t *Msg);
void EPS_P31U_ConfigCmd(const EPS_P31U_ConfigCmd_t *Msg);
void EPS_P31U_GetConfig2Cmd(const EPS_P31U_GetConfig2Cmd_t *Msg);
void EPS_P31U_SetConfig2Cmd(const EPS_P31U_SetConfig2Cmd_t *Msg);
void EPS_P31U_Config2Cmd(const EPS_P31U_Config2Cmd_t *Msg);
void EPS_P31U_SetConfig3Cmd(const EPS_P31U_SetConfig3Cmd_t *Msg);

void EPS_P31U_TransactionCmd(const EPS_P31U_TransactionCmd_t* Msg);

#endif
