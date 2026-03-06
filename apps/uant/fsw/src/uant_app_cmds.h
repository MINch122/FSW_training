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

#ifndef UANT_APP_CMDS_H
#define UANT_APP_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "uant_app_msgstruct.h"
#include "uant_app_msg.h"
#include "gs/util/error.h"


CFE_Status_t UANT_APP_SendBcnCmd(const UANT_APP_SendBcnCmd_t *Msg);
CFE_Status_t UANT_APP_SendHkCmd(const UANT_APP_SendHkCmd_t *Msg);
CFE_Status_t UANT_APP_ResetCountersCmd(const UANT_APP_ResetCountersCmd_t *Msg);
//CFE_Status_t UANT_APP_ProcessCmd(const UANT_APP_ProcessCmd_t *Msg);
CFE_Status_t UANT_APP_NoopCmd(const UANT_APP_NoopCmd_t *Msg);
//CFE_Status_t UANT_APP_DisplayParamCmd(const UANT_APP_DisplayParamCmd_t *Msg);

gs_error_t   UANT_APP_SoftReboot(const UANT_APP_SoftRebootCmd_t *Msg);
gs_error_t   UANT_APP_BurnChannel(const UANT_APP_BurnChannelCmd_t *Msg);
gs_error_t   UANT_APP_StopBurn(const UANT_APP_StopBurnCmd_t *Msg);
gs_error_t   UANT_APP_AutoDeploy(const UANT_APP_AutodeployCmd_t *Msg);

gs_error_t   UANT_APP_GetBoardStatus(const UANT_APP_GetBoardStatusCmd_t *Msg);
gs_error_t   UANT_APP_GetTemperature(const UANT_APP_GetTemperatureCmd_t *Msg);
gs_error_t   UANT_APP_GetReleaseStatus(const UANT_APP_GetStatusCmd_t *Msg);
gs_error_t   UANT_APP_GetBackupStatus(const UANT_APP_GetBackupStatusCmd_t *Msg);
gs_error_t   UANT_APP_GetBackupSettings(const UANT_APP_GetSettingsCmd_t *Msg);
gs_error_t   UANT_APP_SetBackupSettings(const UANT_APP_SetSettingsCmd_t *Msg);

gs_error_t UANT_APP_BurnChannelInternal(const UANT_APP_BurnChannelCmd_t *Msg);
CFE_Status_t UANT_APP_IsReleased(const UANT_RequestRelease_t *Msg);

#endif /* UANT_APP_CMDS_H */
