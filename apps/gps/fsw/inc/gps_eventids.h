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
 * @file  GPS event IDs
 */
#ifndef GPS_EVENTS_H
#define GPS_EVENTS_H

#define GPS_RESERVED_EID    0
#define GPS_INIT_INF_EID    1
#define GPS_CC_ERR_EID      2
#define GPS_NOOP_INF_EID    3
#define GPS_RESET_INF_EID   4
#define GPS_MID_ERR_EID     5
#define GPS_CMD_LEN_ERR_EID 6
#define GPS_PIPE_ERR_EID    7
#define GPS_CR_PIPE_ERR_EID 8
#define GPS_SUB_HK_ERR_EID  9
#define GPS_SUB_CMD_ERR_EID 10
#define GPS_DEV_INF_EID     11
#define GPS_DEV_ERR_EID     12
#define GPS_RX_ERR_EID      13
#define GPS_ADDCB_INF_EID   15
#define GPS_ADDCB_ERR_EID   16
#define GPS_OEM_CMD_ERR_EID 17
#define GPS_HK_RPT_INF_EID  18

#endif /* GPS_EVENTS_H */
