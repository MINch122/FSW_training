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
 *   LTRX Application Topic IDs
 */
#ifndef LTRX_TOPICIDS_H
#define LTRX_TOPICIDS_H

/* Command Topic IDs */
#define CFE_MISSION_LTRX_CMD_TOPICID        0x96
#define CFE_MISSION_LTRX_SEND_HK_TOPICID    0x97
#define CFE_MISSION_LTRX_SEND_BCN_TOPICID   0x98  

/* Telemetry Topic IDs */
#define CFE_MISSION_LTRX_HK_TLM_TOPICID     0x96
#define CFE_MISSION_LTRX_BCN_TLM_TOPICID    0x97  
#define CFE_MISSION_LTRX_RPT_TLM_TOPICID    0x98

#endif /* LTRX_TOPICIDS_H */
