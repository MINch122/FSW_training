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

#ifndef ADCS_CMDS_H
#define ADCS_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "adcs_msg.h"

CFE_Status_t ADCS_SendHkCmd(const ADCS_SendHkCmd_t *Msg);
CFE_Status_t ADCS_SendBcnCmd(const ADCS_SendBcnCmd_t *Msg);
CFE_Status_t ADCS_NoopCmd(const ADCS_NoopCmd_t *Msg);
CFE_Status_t ADCS_ResetCountersCmd(const ADCS_ResetCountersCmd_t *Msg);

CFE_Status_t ADCS_EN_HighCmd(void);
CFE_Status_t ADCS_EN_LowCmd(void);
CFE_Status_t ADCS_Boot_HighCmd(void);
CFE_Status_t ADCS_Boot_LowCmd(void);
CFE_Status_t ADCS_ExitBootloader(void);



/*******************************************
 * 
 * Declaration of Invoked Cmd function
 * Upper functions are just references
 * 
 *********************************************/
/* Set function */
CFE_Status_t ADCS_SetReset(void);
CFE_Status_t ADCS_SetCurrentUnixTimeCmd(ADCS_CurrentUnixTimeCmd_t *msg);
CFE_Status_t ADCS_SetControlEstimationModeCmd(ADCS_ControlEstimationModeCmd_t *msg);
CFE_Status_t ADCS_SetReferenceLLHTargetCmd(ADCS_ReferenceLLHTargetCmd_t *msg);
CFE_Status_t ADCS_SetOrbitModeCmd(ADCS_OrbitModeCmd_t *msg);
CFE_Status_t ADCS_SetReferenceRPYValuesCmd(ADCS_ReferenceRPYvaluesCmd_t *msg);
CFE_Status_t ADCS_SetSatOrbitParamConfigCmd(ADCS_SatOrbitParamConfigCmd_t *msg);
/* Get function */
CFE_Status_t ADCS_GetCurrentUnixTimeCmd(void);
CFE_Status_t ADCS_GetControlEstimationModeCmd(void);
CFE_Status_t ADCS_GetReferenceLLHTargetCmd(void);
CFE_Status_t ADCS_GetOrbitModeCmd(void);
CFE_Status_t ADCS_GetRawCubeSenseSunCmd(void);
CFE_Status_t ADCS_GetPowerStateCmd(void);
CFE_Status_t ADCS_GetSatOrbitParamConfigCmd(void);
CFE_Status_t ADCS_GetRawCSSSensorCmd(void);
CFE_Status_t ADCS_GetRawGYRSensorCmd(void);
CFE_Status_t ADCS_GetCalibratedGYRSensorCmd(void);

#endif /* ADCS_CMDS_H */
