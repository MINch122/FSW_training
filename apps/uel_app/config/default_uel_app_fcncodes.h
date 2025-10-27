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
 *   Specification for the UEL_APP command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef UEL_APP_FCNCODES_H
#define UEL_APP_FCNCODES_H

#define UEL_APP_NOOP_CC                     0
#define UEL_APP_RESET_COUNTERS_CC           1
#define UEL_APP_SEND_BCN_CC                 2
#define UEL_APP_GET_SENS_DATA_CC            3
#define UEL_APP_SET_CAM_SHOT_CC             4
#define UEL_APP_GET_CAM_SHOT_STATUS_CC      5
#define UEL_APP_GET_CAM_IMAGE_CC            6
#define UEL_APP_SET_MOTOR_MODE_CC           7
#define UEL_APP_SET_CAM_POWER_ON_CC         8
#define UEL_APP_SET_CAM_POWER_OFF_CC        9
#define UEL_APP_SET_TERMINAL_CC             10

#define UEL_APP_DOWNLOAD_IMG_CC             23

#endif /* UEL_APP_FCNCODES_H */