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
 *   PAYUEL_ROMA Application Message IDs
 */
#ifndef PAYUEL_ROMA_MSGIDS_H
#define PAYUEL_ROMA_MSGIDS_H

#include "cfe_core_api_base_msgids.h"
#include "payuel_roma_topicids.h"

// Commands
#define PAYUEL_ROMA_CMD_MID           CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_PAYUEL_ROMA_CMD_TOPICID)
#define PAYUEL_ROMA_SEND_HK_MID       CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_PAYUEL_ROMA_SEND_HK_TOPICID)

// Telemetry
#define PAYUEL_ROMA_HK_TLM_MID        CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_PAYUEL_ROMA_HK_TLM_TOPICID)
#define PAYUEL_ROMA_REPORT_TLM_MID    CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_PAYUEL_ROMA_REPORT_TLM_TOPICID)


#endif
