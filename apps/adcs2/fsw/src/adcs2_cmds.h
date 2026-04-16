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
 *   This file contains the prototypes for the Adcs App Ground Command-handling functions
 */

#ifndef ADCS2_CMDS_H
#define ADCS2_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "adcs2_msg.h"

CFE_Status_t ADCS2_NoopCmd(const ADCS2_NoopCmd_t *Msg);
CFE_Status_t ADCS2_ResetCountersCmd(const ADCS2_ResetCountersCmd_t *Msg);

/*******************************************
 * 
 * Declaration of Invoked Cmd function
 * Upper functions are just references
 * 
 *********************************************/
/* Set function */
CFE_Status_t ADCS2_SetReset(void);	// 1
CFE_Status_t ADCS2_SetCurrentUnixTimeCmd(const ADCS2_CurrentUnixTimeCmd_t *msg);	// 2
CFE_Status_t ADCS2_SetPersistConfigCmd(const ADCS2_PersistConfigCmd_t *msg);		// 7
CFE_Status_t ADCS2_SetControlEstimationModeCmd(const ADCS2_ControlEstimationModeCmd_t *msg);	// 42
CFE_Status_t ADCS2_SetPowerStateCmd(const ADCS2_PowerStateCmd_t *msg);	// 56
CFE_Status_t ADCS2_SetMountingConfigCmd(const ADCS2_MountingConfigCmd_t *msg);		// 65

/* Get function */
CFE_Status_t ADCS2_GetCurrentUnixTimeCmd(void);	// 133
CFE_Status_t ADCS2_GetControlEstimationModeCmd(void);	// 150
CFE_Status_t ADCS2_GetRawMAGSensorCmd(void);	// 180
CFE_Status_t ADCS2_GetPowerStateCmd(void);		// 183
CFE_Status_t ADCS2_GetMountingConfigCmd(void);	// 193
CFE_Status_t ADCS2_GetRawGYRSensorCmd(void);	// 204

/* Commissioning sequence */
 CFE_Status_t ADCS2_Comm01Cmd(const ADCS2_Comm01Cmd_t *msg);
 CFE_Status_t ADCS2_Comm02Cmd(const ADCS2_Comm02Cmd_t *msg);
 CFE_Status_t ADCS2_Comm03Cmd(const ADCS2_Comm03Cmd_t *msg);



#endif /* ADCS2_CMDS_H */
