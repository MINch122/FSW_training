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
#include "uant_app_msg.h"

CFE_Status_t UANT_APP_SendHkCmd(const UANT_APP_SendHkCmd_t *Msg);
CFE_Status_t UANT_APP_SendBcnCmd(const UANT_APP_SendBcnCmd_t *Msg);
CFE_Status_t UANT_APP_ResetCountersCmd(const UANT_APP_ResetCountersCmd_t *Msg);
CFE_Status_t UANT_APP_NoopCmd(const UANT_APP_NoopCmd_t *Msg);

CFE_Status_t UANT_APP_Reset(const UANT_APP_ISIS_ResetCmd_t *Msg);
CFE_Status_t UANT_APP_Arm(const UANT_APP_ISIS_ArmAntennaSystemsCmd_t *Msg);
CFE_Status_t UANT_APP_Disarm(const UANT_APP_ISIS_DisarmCmd_t *Msg);
CFE_Status_t UANT_APP_AutomatedDeployment(const UANT_APP_ISIS_AutomatedDeploymentCmd_t *Msg);

CFE_Status_t UANT_APP_DeployAnt1(const UANT_APP_ISIS_DeployAnt1Cmd_t *Msg);
CFE_Status_t UANT_APP_DeployAnt2(const UANT_APP_ISIS_DeployAnt2Cmd_t *Msg);
CFE_Status_t UANT_APP_DeployAnt3(const UANT_APP_ISIS_DeployAnt3Cmd_t *Msg);
CFE_Status_t UANT_APP_DeployAnt4(const UANT_APP_ISIS_DeployAnt4Cmd_t *Msg);

CFE_Status_t UANT_APP_DeployAnt1_Override(const UANT_APP_ISIS_DeployAnt1OverrideCmd_t *Msg);
CFE_Status_t UANT_APP_DeployAnt2_Override(const UANT_APP_ISIS_DeployAnt2OverrideCmd_t *Msg);
CFE_Status_t UANT_APP_DeployAnt3_Override(const UANT_APP_ISIS_DeployAnt3OverrideCmd_t *Msg);
CFE_Status_t UANT_APP_DeployAnt4_Override(const UANT_APP_ISIS_DeployAnt4OverrideCmd_t *Msg);

CFE_Status_t UANT_APP_CancleDeployment(const UANT_APP_ISIS_CancelDeploymentActivationCmd_t *Msg);
CFE_Status_t UANT_APP_GetDeploymentStatus(const UANT_APP_ISIS_ReportDeploymentStatusCmd_t *Msg);
CFE_Status_t UANT_APP_MeasureAntSystemTemperature(const UANT_APP_ISIS_MeasureSystemTemperatureCmd_t *Msg);
CFE_Status_t UANT_APP_ReportAntActivationCnt(const UANT_APP_ISIS_ReportAntActivationCntCmd_t *Msg);
CFE_Status_t UANT_APP_ReportAntActivationTime(const UANT_APP_ISIS_ReportAntActivationTimeCmd_t *Msg);


#endif /* UANT_APP_CMDS_H */
