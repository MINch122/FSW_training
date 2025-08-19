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
 *   This file contains the prototypes for the Sample App Ground Command-handling functions
 */

#ifndef SANT_CMDS_H
#define SANT_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "sant_msgstruct.h"
#include "sant_msg.h"

// #define SANT_I2C_ADDR 0x05


CFE_Status_t SANT_SendHkCmd(const SANT_SendHkCmd_t *Msg);
CFE_Status_t SANT_SendBcnCmd(const SANT_SendBcnCmd_t *Msg);
CFE_Status_t SANT_SendOpCmd(const SANT_SendOpCmd_t *Msg);


CFE_Status_t SANT_NoopCmd(const SANT_NoopCmd_t *Msg);
CFE_Status_t SANT_ResetCountersCmd(const SANT_ResetCountersCmd_t *Msg);


CFE_Status_t SANT_SoftRebootCmd(const SANT_SoftRebootCmd_t *Msg);
CFE_Status_t SANT_BurnCmd(const SANT_BurnCmd_t *Msg);
CFE_Status_t SANT_StopBurnCmd(const SANT_StopBurnCmd_t *Msg);
CFE_Status_t SANT_GetBoardStatusCmd(const SANT_GetBoardStatusCmd_t *Msg);
CFE_Status_t SANT_GetTemperatureCmd(const SANT_GetTemperatureCmd_t *Msg);
CFE_Status_t SANT_GetStatusCmd(const SANT_GetStatusCmd_t *Msg);
CFE_Status_t SANT_GetBackupStatusCmd(const SANT_GetBackupStatusCmd_t *Msg);
CFE_Status_t SANT_GetBackupSettingsCmd(const SANT_GetSettingsCmd_t *Msg);
CFE_Status_t SANT_SetBackupSettingsCmd(const SANT_SetSettingsCmd_t *Msg);


#endif /* SANT_CMDS_H */
