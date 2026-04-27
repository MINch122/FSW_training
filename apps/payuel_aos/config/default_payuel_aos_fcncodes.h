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
 *   Specification for the PAYUEL_AOS command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef PAYUEL_AOS_FCNCODES_H
#define PAYUEL_AOS_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Payuel Aos command codes
*/
#define PAYUEL_AOS_NOOP_CC           0
#define PAYUEL_AOS_RESET_COUNTERS_CC 1
#define PAYUEL_AOS_RESET_CC           2
#define PAYUEL_AOS_WRITE_REGISTER_CC  3
#define PAYUEL_AOS_READ_REGISTER_CC   4
#define PAYUEL_AOS_READ_ALL_CHANNELS_CC 5


#endif
