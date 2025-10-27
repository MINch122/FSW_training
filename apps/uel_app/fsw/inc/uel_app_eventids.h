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
 * Define UEL Events IDs
 */

#ifndef UEL_APP_EVENTS_H
#define UEL_APP_EVENTS_H

#define UEL_APP_RESERVED_EID      0
#define UEL_APP_INIT_INF_EID      1
#define UEL_APP_CC_ERR_EID        2
#define UEL_APP_NOOP_INF_EID      3
#define UEL_APP_RESET_INF_EID     4
#define UEL_APP_MID_ERR_EID       5
#define UEL_APP_CMD_LEN_ERR_EID   6
#define UEL_APP_PIPE_ERR_EID      7
#define UEL_APP_VALUE_INF_EID     8
#define UEL_APP_CR_PIPE_ERR_EID   9
#define UEL_APP_SUB_HK_ERR_EID    10
#define UEL_APP_SUB_CMD_ERR_EID   11
#define UEL_APP_TABLE_REG_ERR_EID 12
#define UEL_APP_TX_ERR_EID        13

#define UEL_APP_CAM_EVT_TASK_ERR_EID 15
#define UEL_APP_CAM_EVT_RECV_ERR_EID   16
#define UEL_APP_CAM_SENSE_RCV_EID      17
#define UEL_APP_CAM_EVT_BOOT_DONE_EID  18
#define UEL_APP_CAM_EVT_SHOT_DONE_EID  19
#define UEL_APP_CAMBOOT_INF_EID        20
#define UEL_APP_TAKEPHOTO_INF_EID      21
#define UEL_APP_IMAGEREQ_INF_EID       22
#define UEL_APP_COMMAND_ERR_EID        23
#define CFE_STATUS_FILE_READ_ERROR     24

#define UEL_APP_DOWNLOAD_DONE_INF_EID   27


#endif /* UEL_APP_EVENTS_H */
