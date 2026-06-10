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
 * TTC Application Platform Configuration Header File
 *
 * This is a compatibility header for the "platform_cfg.h" file that has
 * traditionally provided both public and private config definitions
 * for each CFS app.
 *
 * These definitions are now provided in two separate files, one for
 * the public/mission scope and one for internal scope.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef TTC_INTERNAL_CFG_H
#define TTC_INTERNAL_CFG_H

#include "ttc_internal_cfg_values.h"

/***********************************************************************/
#define TTC_PLATFORM_PIPE_DEPTH         TTC_PLATFORM_CFGVAL(PIPE_DEPTH)
#define DEFAULT_TTC_PLATFORM_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

#define TTC_PLATFORM_PIPE_NAME          TTC_PLATFORM_CFGVAL(PIPE_NAME)
#define DEFAULT_TTC_PLATFORM_PIPE_NAME "TTC_CMD_PIPE"

#define TTC_PLATFORM_MAX_COMMAND_ENTRIES    TTC_PLATFORM_CFGVAL(MAX_COMMAND_ENTRIES)
#define DEFAULT_TTC_PLATFORM_MAX_COMMAND_ENTRIES 128 /* Maximum number of command entries in the timeline */

#define TTC_PLATFORM_MAX_COMMAND_SIZE   TTC_PLATFORM_CFGVAL(MAX_COMMAND_SIZE)
#define DEFAULT_TTC_PLATFORM_MAX_COMMAND_SIZE 128 /* Maximum size of a command in bytes */

#define TTC_PLATFORM_MAX_CHUNK_SIZE     TTC_PLATFORM_CFGVAL(MAX_CHUNK_SIZE)
#define DEFAULT_TTC_PLATFORM_MAX_CHUNK_SIZE 128 /* Maximum bytes per plumb-write chunk */





#define TTC_PLATFORM_NUMBER_OF_TABLES         TTC_PLATFORM_CFGVAL(NUMBER_OF_TABLES)
#define DEFAULT_TTC_PLATFORM_NUMBER_OF_TABLES 1 /* Number of Example Table(s) */

#define TTC_PLATFORM_TABLE_OUT_OF_RANGE_ERR_CODE         TTC_PLATFORM_CFGVAL(TABLE_OUT_OF_RANGE_ERR_CODE)
#define DEFAULT_TTC_PLATFORM_TABLE_OUT_OF_RANGE_ERR_CODE -1

#define TTC_PLATFORM_TBL_ELEMENT_1_MAX         TTC_PLATFORM_CFGVAL(TBL_ELEMENT_1_MAX)
#define DEFAULT_TTC_PLATFORM_TBL_ELEMENT_1_MAX 10

#define TTC_PLATFORM_TABLE_FILE         TTC_PLATFORM_CFGVAL(TABLE_FILE)
#define DEFAULT_TTC_PLATFORM_TABLE_FILE "/cf/ttc_tbl.tbl"

/**
 * Anonymous entry ID pair.
 *
 * When both EntryId and GroupId equal these values the duplicate-ID check is
 * skipped on insertion, and the entry cannot be addressed by ID afterward
 * (delete-by-entry and delete-by-group will not match it).  Use this when
 * the operator does not need to reference the entry after upload, or during
 * testing.  The plumb (chunked-upload) path does not support anonymous entries
 * because write and finalize steps require an addressable ID to locate the
 * reserved slot.
 */
#define TTC_PLATFORM_ANONYMOUS_ENTRY_ID         TTC_PLATFORM_CFGVAL(ANONYMOUS_ENTRY_ID)
#define DEFAULT_TTC_PLATFORM_ANONYMOUS_ENTRY_ID 0

#define TTC_PLATFORM_ANONYMOUS_GROUP_ID         TTC_PLATFORM_CFGVAL(ANONYMOUS_GROUP_ID)
#define DEFAULT_TTC_PLATFORM_ANONYMOUS_GROUP_ID 0

#endif
