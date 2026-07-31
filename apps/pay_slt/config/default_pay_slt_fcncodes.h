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
 *   Specification for the PAY_SLT command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef PAY_SLT_FCNCODES_H
#define PAY_SLT_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** PAY_SLT App command codes
*/
#define PAY_SLT_NOOP_CC                 0
#define PAY_SLT_RESET_COUNTERS_CC       1
#define PAY_SLT_REPORT_BCN_CC           2
#define PAY_SLT_OUTPUT_ENABLED_CC       3

#define PAY_SLT_RS422_PING_CC           4
#define PAY_SLT_PAR_GET_CC              5
#define PAY_SLT_PAR_SET_CC              6
#define PAY_SLT_SCAN_FILES_CC           7
#define PAY_SLT_DOWNLOAD_FILE_I2C_CC    8
#define PAY_SLT_DOWNLOAD_FILE_RS422_CC  9
#define PAY_SLT_GET_FULL_TABLE_CC       10

#endif /* PAY_SLT_FCNCODES_H */
