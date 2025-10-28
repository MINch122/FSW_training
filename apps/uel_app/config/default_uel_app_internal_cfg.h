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
 *   UEL_APP Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef UEL_APP_INTERNAL_CFG_H
#define UEL_APP_INTERNAL_CFG_H

/***********************************************************************/
#define UEL_APP_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

#define UEL_APP_NUMBER_OF_TABLES 1 /* Number of Example Table(s) */

#define UEL_APP_TABLE_OUT_OF_RANGE_ERR_CODE -1

#define UEL_APP_TBL_ELEMENT_1_MAX 10

#define UEL_APP_MUTEX_NAME      "UEL_MUTEX"
#define UEL_APP_CHILD_NAME      "UEL_DOWNLOAD"
#define UEL_APP_CHILD_STACK_SIZE(n)     (4096*(n))
#define UEL_APP_CHILD_PRIORITY          100

#define UEL_APP_IMG_PATH        "/cf/sdcard/uel"
#define UEL_APP_IMG_DATA_SIZE   251

#endif
