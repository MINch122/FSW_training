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
 * Define EPS App Events IDs
 */

#ifndef EPS_EVENTS_H
#define EPS_EVENTS_H

#define EPS_RESERVED_EID        0
#define EPS_CMD_ERR_EID         1
#define EPS_INIT_INF_EID        2
#define EPS_NOOP_INF_EID        3
#define EPS_RESET_INF_EID       4
#define EPS_PIPE_ERR_EID        5
#define EPS_CR_PIPE_ERR_EID     6
#define EPS_SUB_HK_ERR_EID     7
#define EPS_SUB_CMD_ERR_EID    8
#define EPS_CMD_LEN_ERR_EID    9
#define EPS_MID_ERR_EID        10

/* BP8 Battery Pack event IDs */
#define EPS_BP8_HK_INF_EID        20
#define EPS_BP8_HEATER_INF_EID    21
#define EPS_BP8_FAULT_INF_EID     22

#endif
