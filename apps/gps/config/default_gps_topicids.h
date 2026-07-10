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
 *   GPS Application Topic IDs
 */
#ifndef DEFAULT_GPS_TOPICIDS_H
#define DEFAULT_GPS_TOPICIDS_H

/* MID allocation per wiki (NURI 5th_MID): GPS HK_TLM 0x0894, REPORT 0x0895 */
#define CFE_MISSION_GPS_CMD_TOPICID         0x94  /* MID 0x1894 */
#define CFE_MISSION_GPS_SEND_HK_TOPICID     0x95  /* MID 0x1895 */
#define CFE_MISSION_GPS_HK_TLM_TOPICID      0x94  /* MID 0x0894 */
#define CFE_MISSION_GPS_REPORT_TLM_TOPICID  0x95  /* MID 0x0895 */

#endif
