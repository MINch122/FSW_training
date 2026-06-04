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
 *   Specification for the LTRX command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions. Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 *
 * Reference: Beacon OBC ICD Rev 3, Page 9 - Command type lookup table
 */
#ifndef LTRX_FCNCODES_H
#define LTRX_FCNCODES_H

/* Basic app management CC */
#define LTRX_NOOP_CC                      0  
#define LTRX_RESET_COUNTERS_CC            1  
#define LTRX_RESET_APP_CMD_COUNTERS_CC    2  
#define LTRX_RESET_DEVICE_CMD_COUNTERS_CC 3  
#define LTRX_SEND_STATUS_CC               5  

/* Child/Session request trigger */
#define LTRX_SESSION_START_DOWNLINK_CC    10  // Start downlink
#define LTRX_SESSION_ABORT_CC             11  // Abort session
#define LTRX_SESSION_RESET_STATE_CC       12  // Reset session

#define LTRX_QUERY_BEACON_STATUS_CC       30 // Type ID 23
#define LTRX_QUERY_GNSS_INFO_CC           31 // Type ID 21

/* Downstream gating */
#define LTRX_DOWNSTREAM_ENABLE_CC         40
#define LTRX_DOWNSTREAM_DISABLE_CC        41

#define LTRX_TEST_CSP_PING_CC            50 // can test

#endif /* LTRX_FCNCODES_H */