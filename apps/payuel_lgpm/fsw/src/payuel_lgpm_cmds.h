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
 *   This file contains the prototypes for the PAYUEL_LGPM App Ground Command-handling functions
 */

#ifndef PAYUEL_LGPM_CMDS_H
#define PAYUEL_LGPM_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "payuel_lgpm_msg.h"

CFE_Status_t PAYUEL_LGPM_SendHkCmd(const PAYUEL_LGPM_SendHkCmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_ResetCountersCmd(const PAYUEL_LGPM_ResetCountersCmd_t *Msg);
// CFE_Status_t PAYUEL_LGPM_ProcessCmd(const PAYUEL_LGPM_ProcessCmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_NoopCmd(const PAYUEL_LGPM_NoopCmd_t *Msg);
// CFE_Status_t PAYUEL_LGPM_DisplayParamCmd(const PAYUEL_LGPM_DisplayParamCmd_t *Msg);
// CFE_Status_t PAYUEL_LGPM_NativeCANCmd(const PAYUEL_LGPM_NativeCANCmd_t *Msg);

CFE_Status_t PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd(const PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_3V3_PWR_ON_Cmd(const PAYUEL_LGPM_3V3_PWR_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_3V3_PWR_OFF_Cmd(const PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd(const PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd(const PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd(const PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd(const PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_V28_MAIN_ON_Cmd(const PAYUEL_LGPM_V28_MAIN_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_V28_MAIN_OFF_Cmd(const PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_V28_SUB_ON_Cmd(const PAYUEL_LGPM_V28_SUB_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_V28_SUB_OFF_Cmd(const PAYUEL_LGPM_V28_SUB_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_V12_MAIN_ON_Cmd(const PAYUEL_LGPM_V12_MAIN_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_V12_MAIN_OFF_Cmd(const PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_PWR_SENSE_INFO_Cmd(const PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_PWR_SEQ_ON_Cmd(const PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_PWR_SEQ_OFF_Cmd(const PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_RWA_CONTROL_Cmd(const PAYUEL_LGPM_RWA_CONTROL_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_RWA_PWR_ON_Cmd(const PAYUEL_LGPM_RWA_PWR_ON_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_RWA_PWR_OFF_Cmd(const PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t *Msg);
CFE_Status_t PAYUEL_LGPM_RWA_SENSE_INFO_Cmd(const PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t *Msg);

#endif /* PAYUEL_LGPM_CMDS_H */
