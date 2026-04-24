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
 *   Specification for the UANT_APP_APP command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef UANT_APP_APP_FCNCODES_H
#define UANT_APP_APP_FCNCODES_H

// }uant_app_cc_n;

#define    UANT_APP_NOOP_CC 0         
#define    UANT_APP_RESET_COUNTERS_CC 1   
#define    UANT_APP_SOFT_REBOOT_CC 2      

    /* Burn control */
#define    UANT_APP_BURN_CHANNEL_CC   3      
#define   UANT_APP_STOP_BURN_CC   4        

    /* Telemetry */
#define    UANT_APP_GET_STATUS_CC    5        
#define   UANT_APP_GET_BACKUP_STATUS_CC 6 
#define   UANT_APP_GET_BOARD_STATUS_CC 7  
#define    UANT_APP_GET_TEMPERATURE_CC 8  
#define    UANT_APP_GET_SETTINGS_CC 9     
#define   UANT_APP_SET_SETTINGS_CC  10

#define UANT_BURN_CHANNEL_INTERNAL_CC    23
#define UANT_GET_STATUS_INTERNAL_CC      24


#endif

