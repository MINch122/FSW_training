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
 *   STRX Application Topic IDs
 */
#ifndef STRX_TOPICIDS_H
#define STRX_TOPICIDS_H

#define CFE_MISSION_STRX_CMD_TOPICID       0x54 //1850
#define CFE_MISSION_STRX_SEND_HK_TOPICID   0x55 //1851
#define CFE_MISSION_STRX_SEND_BCN_TOPICID  0x57 //1853

#define CFE_MISSION_STRX_HK_TLM_TOPICID    0x52 //0850
#define CFE_MISSION_STRX_BCN_TLM_TOPICID   0x53 //0851
#define CFE_MISSION_STRX_RPT_TLM_TOPICID   0x56 //0860 

#endif
