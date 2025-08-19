
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

#ifndef SANT_EVENTS_H
#define SANT_EVENTS_H

#define SANT_RESERVED_EID      0
#define SANT_INIT_INF_EID      1
#define SANT_CC_ERR_EID        2
#define SANT_NOOP_INF_EID      3
#define SANT_RESET_INF_EID     4
#define SANT_MID_ERR_EID       5
#define SANT_CMD_LEN_ERR_EID   6
#define SANT_PIPE_ERR_EID      7
#define SANT_VALUE_INF_EID     8
#define SANT_CR_PIPE_ERR_EID   9
#define SANT_SUB_HK_ERR_EID    10
#define SANT_SUB_CMD_ERR_EID   11
#define SANT_TABLE_REG_ERR_EID 12

/* ───── Error Events: Command Execution Failures ───── */
#define SANT_I2C_XFER_ERR_EID      13  /* I2C 트랜잭션 실패 */
#define SANT_BURN_ERR_EID          14  /* Burn 명령 실패 */
#define SANT_STOP_BURN_ERR_EID     15  /* Stop-burn 실패 */
#define SANT_GET_BOARD_ERR_EID     16  /* Board-status 조회 실패 */
#define SANT_GET_TEMP_ERR_EID      17  /* Temperature 조회 실패 */
#define SANT_GET_STATUS_ERR_EID    18  /* Release-status 조회 실패 */
#define SANT_GET_BACKUP_ERR_EID    19  /* Backup-status 조회 실패 */
#define SANT_GET_SETTINGS_ERR_EID  20  /* Backup-settings 읽기 실패 */
#define SANT_SET_SETTINGS_ERR_EID  21  /* Backup-settings 쓰기 실패 */
#define SANT_I2C_INIT_ERR_EID      22


#endif /* SANT_EVENTS_H */
