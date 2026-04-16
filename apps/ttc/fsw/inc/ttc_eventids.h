/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 * Define Ttc Events IDs
 */

#ifndef TTC_EVENTS_H
#define TTC_EVENTS_H

/**
 * Generic Application event IDs.
 */
#define TTC_RESERVED_EID                0
#define TTC_INIT_INF_EID                1
#define TTC_CC_ERR_EID                  2
#define TTC_NOOP_INF_EID                3
#define TTC_RESET_INF_EID               4
#define TTC_MID_ERR_EID                 5
#define TTC_CMD_LEN_ERR_EID             6
#define TTC_PIPE_ERR_EID                7
#define TTC_VALUE_INF_EID               8
#define TTC_CR_PIPE_ERR_EID             9
#define TTC_SUB_HK_ERR_EID              10
#define TTC_SUB_CMD_ERR_EID             11
#define TTC_SUB_WAKEUP_ERR_EID          12
#define TTC_TABLE_REG_ERR_EID           13

/**
 * TTC-specific event IDs.
 */
#define TTC_PENDING_COUNT_INF_EID       20
#define TTC_NEXT_ENTRY_ID_INF_EID       21
#define TTC_NEXT_EXEC_TIME_INF_EID      22
#define TTC_RESET_TIMELINE_HK_INF_EID   23
#define TTC_INSERT_CMD_ERR_EID          24
#define TTC_DELETE_ENTRY_ERR_EID        25
#define TTC_DELETE_ENTRY_INF_EID        26
#define TTC_DELETE_GROUP_INF_EID        27
#define TTC_DELETE_ALL_ENTRIES_INF_EID  28
#define TTC_EXECUTE_ENTRY_INF_EID       29
#define TTC_EXECUTE_ENTRY_ERR_EID       30
#define TTC_EXECUTE_GROUP_INF_EID       31
#define TTC_EXECUTE_GROUP_ERR_EID       32
#define TTC_PAUSE_EXECUTION_INF_EID     33
#define TTC_RESUME_EXECUTION_INF_EID    34
#define TTC_PLUMB_INIT_ERR_EID          35
#define TTC_PLUMB_WRITE_ERR_EID         36
#define TTC_PLUMB_FINALIZE_ERR_EID      37
#define TTC_PLUMB_DELETE_RESERVED_ERR_EID 38
#define TTC_PLUMB_PURGE_INF_EID         39

/**
 * Timeline execution event IDs. Automatically emitted by the timeline
 * execution engine.
 */
#define TTC_TIME_JUMP_FORWARD_INF_EID   40
#define TTC_TIME_JUMP_BACKWARD_INF_EID  41
#define TTC_ABORTING_ENTRIES_INF_EID    42
#define TTC_SKIPPING_ENTRY_INF_EID      43
#define TTC_INVALID_EXEC_TYPE_ERR_EID   44
#define TTC_INVALID_NEXT_SLOT_ERR_EID   45
#define TTC_INVALID_ENTRY_STATUS_ERR_EID 46

#endif /* TTC_EVENTS_H */
