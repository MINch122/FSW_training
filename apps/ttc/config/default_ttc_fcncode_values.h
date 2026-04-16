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
 *   Specification for the CFE Executive Services (CFE_ES) command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef DEFAULT_TTC_FCNCODE_VALUES_H
#define DEFAULT_TTC_FCNCODE_VALUES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

#define TTC_CCVAL(x) TTC_FunctionCode_##x

enum TTC_FunctionCode_ {
    TTC_FunctionCode_NOOP                  =  0,
    TTC_FunctionCode_RESET_COUNTERS        =  1,
    TTC_FunctionCode_REPORT                =  2,

    TTC_FunctionCode_GET_TIMELINE_HK       = 10,
    TTC_FunctionCode_RESET_TIMELINE_HK     = 11,

    TTC_FunctionCode_GET_PENDING_ENTRY_COUNT = 12,
    TTC_FunctionCode_GET_NEXT_ENTRY_ID       = 13,
    TTC_FunctionCode_GET_NEXT_EXEC_TIME     = 14,

    TTC_FunctionCode_INSERT_ABS_CMD_ENTRY  = 15,
    TTC_FunctionCode_INSERT_REL_CMD_ENTRY  = 16,

    TTC_FunctionCode_DELETE_CMD_ENTRY      = 17,
    TTC_FunctionCode_DELETE_CMD_GROUP      = 18,
    TTC_FunctionCode_DELETE_CMD_ENTRIES    = 19,

    TTC_FunctionCode_EXECUTE_ENTRY         = 20,
    TTC_FunctionCode_EXECUTE_GROUP         = 21,

    TTC_FunctionCode_PAUSE_TIMELINE_PROCESSING  = 22,
    TTC_FunctionCode_RESUME_TIMELINE_PROCESSING = 23,

    TTC_FunctionCode_PLUMB_INIT_ENTRY      = 24,
    TTC_FunctionCode_PLUMB_WRITE_ENTRY     = 25,
    TTC_FunctionCode_PLUMB_FINALIZE_ENTRY  = 26,
    TTC_FunctionCode_PLUMB_PURGE_TIMELINE  = 27,
    TTC_FunctionCode_PLUMB_DELETE_RESERVED_ENTRY = 28
};

#endif
