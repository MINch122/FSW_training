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
 *   Specification for the TTC command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef TTC_FCNCODES_H
#define TTC_FCNCODES_H

#include "ttc_fcncode_values.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Ttc command codes
*/
#define TTC_NOOP_CC                 TTC_CCVAL(NOOP)
#define TTC_RESET_COUNTERS_CC       TTC_CCVAL(RESET_COUNTERS)
#define TTC_REPORT_CC               TTC_CCVAL(REPORT)

#define TTC_GET_TIMELINE_HK_CC       TTC_CCVAL(GET_TIMELINE_HK)
#define TTC_RESET_TIMELINE_HK_CC     TTC_CCVAL(RESET_TIMELINE_HK)

#define TTC_GET_PENDING_ENTRY_COUNT_CC   TTC_CCVAL(GET_PENDING_ENTRY_COUNT)
#define TTC_GET_NEXT_ENTRY_ID_CC         TTC_CCVAL(GET_NEXT_ENTRY_ID)
#define TTC_GET_NEXT_EXEC_TIME_CC        TTC_CCVAL(GET_NEXT_EXEC_TIME)

#define TTC_INSERT_ABS_CMD_ENTRY_CC TTC_CCVAL(INSERT_ABS_CMD_ENTRY)
#define TTC_INSERT_REL_CMD_ENTRY_CC TTC_CCVAL(INSERT_REL_CMD_ENTRY)

#define TTC_DELETE_CMD_ENTRY_CC     TTC_CCVAL(DELETE_CMD_ENTRY)
#define TTC_DELETE_CMD_GROUP_CC     TTC_CCVAL(DELETE_CMD_GROUP)
#define TTC_DELETE_ALL_ENTRIES_CC   TTC_CCVAL(DELETE_CMD_ENTRIES)

#define TTC_EXECUTE_CMD_ENTRY_CC    TTC_CCVAL(EXECUTE_ENTRY)
#define TTC_EXECUTE_CMD_GROUP_CC    TTC_CCVAL(EXECUTE_GROUP)

#define TTC_PAUSE_TIMELINE_PROCESSING_CC  TTC_CCVAL(PAUSE_TIMELINE_PROCESSING)
#define TTC_RESUME_TIMELINE_PROCESSING_CC TTC_CCVAL(RESUME_TIMELINE_PROCESSING)

#define TTC_PLUMB_INIT_ENTRY_CC     TTC_CCVAL(PLUMB_INIT_ENTRY)
#define TTC_PLUMB_WRITE_ENTRY_CC    TTC_CCVAL(PLUMB_WRITE_ENTRY)
#define TTC_PLUMB_FINALIZE_ENTRY_CC TTC_CCVAL(PLUMB_FINALIZE_ENTRY)
#define TTC_PLUMB_PURGE_TIMELINE_CC TTC_CCVAL(PLUMB_PURGE_TIMELINE)
#define TTC_PLUMB_DELETE_RESERVED_ENTRY_CC TTC_CCVAL(PLUMB_DELETE_RESERVED_ENTRY)

#endif
