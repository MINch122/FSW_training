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
 *   TTC Application Topic IDs
 */
#ifndef TTC_TOPICIDS_H
#define TTC_TOPICIDS_H

#include "ttc_topicid_values.h"

#define TTC_MISSION_CMD_TOPICID             TTC_MISSION_TIDVAL(CMD)
#define DEFAULT_TTC_MISSION_CMD_TOPICID     0x86
#define TTC_MISSION_SEND_HK_TOPICID         TTC_MISSION_TIDVAL(SEND_HK)
#define DEFAULT_TTC_MISSION_SEND_HK_TOPICID 0x87
#define TTC_MISSION_HK_TLM_TOPICID          TTC_MISSION_TIDVAL(HK_TLM)
#define DEFAULT_TTC_MISSION_HK_TLM_TOPICID  0x88
#define TTC_MISSION_ONEHZ_WAKEUP_TOPICID      TTC_MISSION_TIDVAL(ONEHZ_WAKEUP)
#define DEFAULT_TTC_MISSION_ONEHZ_WAKEUP_TOPICID 0x89

#endif
