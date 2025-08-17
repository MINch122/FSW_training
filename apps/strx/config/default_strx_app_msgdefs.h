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
 *   Specification for the STRX_APP command and telemetry
 *   message constant definitions.
 *
 *  For STRX_APP this is only the function/command code definitions
 */
#ifndef STRX_APP_MSGDEFS_H
#define STRX_APP_MSGDEFS_H

#include "common_types.h"
#include "strx_app_fcncodes.h"


/*************************************************************************/
/*
** Type definition (Strx App housekeeping)
*/

// typedef struct STRX_APP_HkTlm_Payload
// {
//     int8_t CommandErrorCounter;
//     int8_t CommandCounter;
//     int8_t spare[2];
// } STRX_APP_HkTlm_Payload_t;


typedef struct
{
    uint32_t rx_freq;
    uint32_t rx_baud;
    uint16_t rx_guard;
    int16_t LastRssi;
    uint16_t BootCount;
    uint32_t BootCause;
    uint32_t TotTxBytes;
    uint32_t TotRxBytes;
    int8_t hw_det;
    uint8_t rxmode;
    uint16_t gnd_wdt_cnt;
    uint32_t gnd_wdt_left;
    uint32_t tx_freq;
    uint32_t tx_baud;
    uint16_t tx_guard;
    int16_t rssibusy;
} STRX_HkTlm_Payload_t;

typedef struct {
    uint32_t BaudRxconf;
    uint32_t BaudTxconf;
    uint16_t GuardRxconf;
} STRX_ConfBitTable_t;

typedef struct
{
    int16_t LastRssi;
    uint16_t BootCount;
    uint32_t BootCause;
    uint8_t rxmode;
    uint16_t gnd_wdt_cnt;
    uint32_t gnd_wdt_left;
    
}__attribute__((packed)) STRX_BcnTlm_Payload_t;


typedef struct STRX_AppCount
{
    uint8_t AppCmdCounter;
    uint8_t AppErrCounter;
    uint8_t DeviceErrCounter;
    
} STRX_AppCount_t;




#endif
