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
 *   Specification for the UTRX command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef SLT_IFB_FCNCODES_H
#define SLT_IFB_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** SLT_IFB App command codes
*/
#define PAY_SLT_NOOP_CC                 0
#define PAY_SLT_RESET_COUNTERS_CC       1
#define PAY_SLT_REPORT_BCN_CC           2
#define PAY_SLT_SET_BCN_ENABLED_CC      3


#define PAY_SLT_PAR_GET_CC              87
#define PAY_SLT_PAR_SET_CC              88
#define PAY_SLT_SCAN_FILES_CC           89
#define PAY_SLT_DOWNLOAD_FILE_CC        90
#define PAY_SLT_GET_FULL_TABLE_CC       91

#define SLT_IFB_NOOP_CC                 PAY_SLT_NOOP_CC
#define SLT_IFB_RESET_COUNTERS_CC       PAY_SLT_RESET_COUNTERS_CC
#define SLT_IFB_SET_BCN_ENABLED_CC      PAY_SLT_SET_BCN_ENABLED_CC


#endif 
