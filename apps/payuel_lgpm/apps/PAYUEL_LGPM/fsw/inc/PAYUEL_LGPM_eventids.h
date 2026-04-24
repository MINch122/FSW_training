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
 * Define PAYUEL_LGPM Events IDs
 */

#ifndef PAYUEL_LGPM_EVENTS_H
#define PAYUEL_LGPM_EVENTS_H

#define PAYUEL_LGPM_RESERVED_EID      0
#define PAYUEL_LGPM_INIT_INF_EID      1
#define PAYUEL_LGPM_CC_ERR_EID        2
#define PAYUEL_LGPM_NOOP_INF_EID      3
#define PAYUEL_LGPM_RESET_INF_EID     4
#define PAYUEL_LGPM_MID_ERR_EID       5
#define PAYUEL_LGPM_CMD_LEN_ERR_EID   6
#define PAYUEL_LGPM_PIPE_ERR_EID      7
#define PAYUEL_LGPM_VALUE_INF_EID     8
#define PAYUEL_LGPM_CR_PIPE_ERR_EID   9
#define PAYUEL_LGPM_SUB_HK_ERR_EID    10
#define PAYUEL_LGPM_SUB_CMD_ERR_EID   11
#define PAYUEL_LGPM_TABLE_REG_ERR_EID 12

#define PAYUEL_LGPM_MCU_CHECK_ALIVE_ERR_EID 13

#endif /* PAYUEL_LGPM_EVENTS_H */
