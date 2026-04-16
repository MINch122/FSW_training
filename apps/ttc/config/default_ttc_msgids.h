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
 *   TTC Application Message IDs
 */
#ifndef DEFAULT_TTC_MSGIDS_H
#define DEFAULT_TTC_MSGIDS_H

#include "cfe_core_api_base_msgids.h"
#include "ttc_msgid_values.h"

#define TTC_CMD_MID     TTC_CMD_PLATFORM_MIDVAL(CMD)
#define TTC_SEND_HK_MID TTC_CMD_PLATFORM_MIDVAL(SEND_HK)
#define TTC_HK_TLM_MID  TTC_TLM_PLATFORM_MIDVAL(HK_TLM)

#define TTC_ONEHZ_WAKEUP_MID TTC_CMD_PLATFORM_MIDVAL(ONEHZ_WAKEUP)

#endif
