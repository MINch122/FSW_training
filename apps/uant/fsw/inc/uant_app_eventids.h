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
 *
 * Define Sample App Events IDs
 */

#ifndef UANT_APP_EVENTS_H
#define UANT_APP_EVENTS_H

#define UANT_APP_RESERVED_EID      0
#define UANT_APP_INIT_INF_EID      1
#define UANT_APP_CC_ERR_EID        2
#define UANT_APP_NOOP_INF_EID      3
#define UANT_APP_RESET_INF_EID     4
#define UANT_APP_MID_ERR_EID       5
#define UANT_APP_CMD_LEN_ERR_EID   6
#define UANT_APP_PIPE_ERR_EID      7
#define UANT_APP_VALUE_INF_EID     8
#define UANT_APP_CR_PIPE_ERR_EID   9
#define UANT_APP_SUB_HK_ERR_EID    10
#define UANT_APP_SUB_CMD_ERR_EID   11
#define UANT_APP_TABLE_REG_ERR_EID 12
#define UANT_APP_INIT_EID          24

/* ───── Error Events: Command Execution Failures ───── */
#define UANT_APP_RESET_ERR_EID          13  /* ISIS_UANT_Reset 실패 */
#define UANT_APP_ARM_ERR_EID            14  /* ISIS_UANT_ArmAntennaSystems 실패 */
#define UANT_APP_DISARM_ERR_EID         15  /* ISIS_UANT_Disarm 실패 */
#define UANT_APP_AUTO_DEPLOY_ERR_EID    16  /* ISIS_UANT_AutomatedSequentialDeployment 실패 */
#define UANT_APP_DEPLOY_ERR_EID         17  /* ISIS_UANT_DeployAntennaX 실패 */
#define UANT_APP_DEPLOY_OVRD_ERR_EID    18  /* ISIS_UANT_DeployAntennaXWithOverride 실패 */
#define UANT_APP_DEPLOY_CANCEL_ERR_EID  19  /* ISIS_UANT_CancelDeploymentSystemActivation 실패 */

#define UANT_APP_GET_STATUS_ERR_EID     20  /* ISIS_UANT_ReportDeploymentStatus 실패 */
#define UANT_APP_MEASURE_TEMP_ERR_EID   21  /* ISIS_UANT_MeasureAntennaSystemTemperature 실패 */
#define UANT_APP_GET_ACT_CNT_ERR_EID    22  /* ISIS_UANT_ReportAntennaActivationCount 실패 */
#define UANT_APP_GET_ACT_TIME_ERR_EID   23  /* ISIS_UANT_ReportAntennaActivationTime 실패 */

#endif /* UANT_APP_EVENTS_H */
