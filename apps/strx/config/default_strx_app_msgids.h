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
 *   STRX_APP Application Message IDs
 */
#ifndef STRX_APP_MSGIDS_H
#define STRX_APP_MSGIDS_H

#include "cfe_core_api_base_msgids.h"
#include "strx_app_topicids.h"

#define STRX_APP_CMD_MID     CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_CMD_TOPICID) //1850
#define STRX_APP_SEND_HK_MID CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_SEND_HK_TOPICID) //1851
#define STRX_APP_OIF_MID     CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_OIF_TOPICID) //1852
#define STRX_APP_SEND_BCN_MID CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_SEND_BCN_TOPICID) //1853
#define STRX_APP_HK_TLM_MID  CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_HK_TLM_TOPICID) //0850
#define STRX_APP_BCN_TLM_MID CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_BCN_TLM_TOPICID) //0851
#define STRX_APP_RPT_TLM_MID CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_STRX_APP_RPT_TLM_TOPICID) 

#endif
