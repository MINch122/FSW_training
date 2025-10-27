/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License; you may obtain
 * a copy at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Prototypes for UEL ground command-handling functions
 */

#ifndef UEL_APP_CMDS_H
#define UEL_APP_CMDS_H

/*
** Required header files
*/
#include "cfe_error.h"   /* CFE_Status_t */
#include "uel_app_msg.h" /* Command message structs */


CFE_Status_t UEL_APP_NoopCmd(const UEL_APP_NoopCmd_t *Msg);
CFE_Status_t UEL_APP_ResetCountersCmd(const UEL_APP_ResetCountersCmd_t *Msg);
CFE_Status_t UEL_APP_SendBcnCmd(const UEL_APP_SendBcnCmd_t *Msg);
CFE_Status_t UEL_APP_GetSensData(const UEL_APP_GetSensDataCmd_t *Msg);
CFE_Status_t UEL_APP_SetCamPowerOnCmd(const UEL_APP_SetCamPowerCmd_t *Msg);
CFE_Status_t UEL_APP_SetCamPowerOffCmd(const UEL_APP_SetCamPowerCmd_t *Msg);
CFE_Status_t UEL_APP_SetCamShotCmd(const UEL_APP_SetCamShotCmd_t *Msg);
CFE_Status_t UEL_APP_GetCamShotStatus(const UEL_APP_GetCamShootStatusCmd_t *Msg);
CFE_Status_t UEL_APP_GetCamImageCmd(const UEL_APP_GetCamImageCmd_t *Msg);
CFE_Status_t UEL_APP_SetTerminalCmd(const UEL_APP_SetTerminalCmd_t *Msg);
CFE_Status_t UEL_APP_SetMotorMode(const UEL_APP_SetMotorMode_t *Msg);
CFE_Status_t UEL_APP_DownloadImgCmd(const UEL_APP_DownloadAllCmd_t *Msg);

#endif /* UEL_APP_CMDS_H */