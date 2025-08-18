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

#ifndef SANT_APP_CMDS_H
#define SANT_APP_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "sant_app_msgstruct.h"
#include "sant_app_msg.h"

// #define SANT_I2C_ADDR 0x05


CFE_Status_t SANT_APP_SendHkCmd(const SANT_APP_SendHkCmd_t *Msg);
CFE_Status_t SANT_APP_SendBcnCmd(const SANT_APP_SendBcnCmd_t *Msg);
CFE_Status_t SANT_APP_SendOpCmd(const SANT_APP_SendOpCmd_t *Msg);


CFE_Status_t SANT_APP_NoopCmd(const SANT_APP_NoopCmd_t *Msg);
CFE_Status_t SANT_APP_ResetCountersCmd(const SANT_APP_ResetCountersCmd_t *Msg);


CFE_Status_t SANT_APP_SoftRebootCmd(const SANT_APP_SoftRebootCmd_t *Msg);
CFE_Status_t SANT_APP_BurnCmd(const SANT_APP_BurnCmd_t *Msg);
CFE_Status_t SANT_APP_StopBurnCmd(const SANT_APP_StopBurnCmd_t *Msg);
CFE_Status_t SANT_APP_GetBoardStatusCmd(const SANT_APP_GetBoardStatusCmd_t *Msg);
CFE_Status_t SANT_APP_GetTemperatureCmd(const SANT_APP_GetTemperatureCmd_t *Msg);
CFE_Status_t SANT_APP_GetStatusCmd(const SANT_APP_GetStatusCmd_t *Msg);
CFE_Status_t SANT_APP_GetBackupStatusCmd(const SANT_APP_GetBackupStatusCmd_t *Msg);
CFE_Status_t SANT_APP_GetBackupSettingsCmd(const SANT_APP_GetSettingsCmd_t *Msg);
CFE_Status_t SANT_APP_SetBackupSettingsCmd(const SANT_APP_SetSettingsCmd_t *Msg);


#endif /* SANT_APP_CMDS_H */
