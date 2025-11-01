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
 * Define GPS App Events IDs
 */

#ifndef GPS_EVENTS_H
#define GPS_EVENTS_H

#define GPS_RESERVED_EID             0
#define GPS_INIT_INF_EID             1
#define GPS_CC_ERR_EID               2
#define GPS_NOOP_INF_EID             3
#define GPS_RESET_INF_EID            4
#define GPS_MID_ERR_EID              5
#define GPS_CMD_LEN_ERR_EID          6
#define GPS_PIPE_ERR_EID             7
#define GPS_CR_PIPE_ERR_EID          9
#define GPS_SUB_HK_ERR_EID           10
#define GPS_SUB_CMD_ERR_EID          11

#define GPS_DEV_TASK_INIT_INF_EID    20
#define GPS_DEV_TASK_RESP_INF_EID    21
#define GPS_DEV_LOG_NOBUF_ERR_EID    22
#define GPS_DEV_LOG_CB_NULL_ERR_EID  23
#define GPS_DEV_TASK_TASKLV_ERR_EID  24
#define GPS_DEV_TASK_CRASH_ERR_EID   25
#define GPS_DEV_HANDLER_INIT_ERR_EID 26
#define GPS_DRIVER_TASK_INIT_ERR_EID 27
#define GPS_DEV_DL_ERR_EID           28

#endif
