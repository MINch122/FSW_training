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

#ifndef UANT_CMDS_H
#define UANT_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "uant_msg.h"

CFE_Status_t UANT_SendHkCmd(const UANT_SendHkCmd_t *Msg);
CFE_Status_t UANT_SendBcnCmd(const UANT_SendBcnCmd_t *Msg);
CFE_Status_t UANT_ResetCountersCmd(const UANT_ResetCountersCmd_t *Msg);
CFE_Status_t UANT_NoopCmd(const UANT_NoopCmd_t *Msg);

CFE_Status_t UANT_Reset(const UANT_ISIS_ResetCmd_t *Msg);
CFE_Status_t UANT_Arm(const UANT_ISIS_ArmAntennaSystemsCmd_t *Msg);
CFE_Status_t UANT_Disarm(const UANT_ISIS_DisarmCmd_t *Msg);
CFE_Status_t UANT_AutomatedDeployment(const UANT_ISIS_AutomatedDeploymentCmd_t *Msg);

CFE_Status_t UANT_DeployAnt1(const UANT_ISIS_DeployAnt1Cmd_t *Msg);
CFE_Status_t UANT_DeployAnt2(const UANT_ISIS_DeployAnt2Cmd_t *Msg);
CFE_Status_t UANT_DeployAnt3(const UANT_ISIS_DeployAnt3Cmd_t *Msg);
CFE_Status_t UANT_DeployAnt4(const UANT_ISIS_DeployAnt4Cmd_t *Msg);

CFE_Status_t UANT_DeployAnt1_Override(const UANT_ISIS_DeployAnt1OverrideCmd_t *Msg);
CFE_Status_t UANT_DeployAnt2_Override(const UANT_ISIS_DeployAnt2OverrideCmd_t *Msg);
CFE_Status_t UANT_DeployAnt3_Override(const UANT_ISIS_DeployAnt3OverrideCmd_t *Msg);
CFE_Status_t UANT_DeployAnt4_Override(const UANT_ISIS_DeployAnt4OverrideCmd_t *Msg);

CFE_Status_t UANT_CancleDeployment(const UANT_ISIS_CancelDeploymentActivationCmd_t *Msg);
CFE_Status_t UANT_GetDeploymentStatus(const UANT_ISIS_ReportDeploymentStatusCmd_t *Msg);
CFE_Status_t UANT_MeasureAntSystemTemperature(const UANT_ISIS_MeasureSystemTemperatureCmd_t *Msg);
CFE_Status_t UANT_ReportAntActivationCnt(const UANT_ISIS_ReportAntActivationCntCmd_t *Msg);
CFE_Status_t UANT_ReportAntActivationTime(const UANT_ISIS_ReportAntActivationTimeCmd_t *Msg);


#endif /* UANT_CMDS_H */
