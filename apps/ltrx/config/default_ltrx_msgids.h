/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
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
 *   LTRX Application Message IDs
 */
#ifndef LTRX_MSGIDS_H
#define LTRX_MSGIDS_H

#include "cfe_core_api_base_msgids.h"
#include "ltrx_topicids.h"


#define LTRX_CMD_MID        CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_LTRX_CMD_TOPICID)
#define LTRX_SEND_HK_MID    CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_LTRX_SEND_HK_TOPICID)
#define LTRX_SEND_BCN_MID   CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_LTRX_SEND_BCN_TOPICID)  


#define LTRX_HK_TLM_MID     CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_LTRX_HK_TLM_TOPICID)
#define LTRX_BCN_TLM_MID    CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_LTRX_BCN_TLM_TOPICID)    
#define LTRX_RPT_TLM_MID    CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_LTRX_RPT_TLM_TOPICID)

#endif /* LTRX_MSGIDS_H */
