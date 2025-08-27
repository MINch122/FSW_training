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
 *   Specification for the UTRX command and telemetry
 *   message constant definitions.
 *
 *  For UTRX this is only the function/command code definitions
 */
#ifndef UTRX_MSGDEFS_H
#define UTRX_MSGDEFS_H

#include "common_types.h"
#include "utrx_fcncodes.h"

/*************************************************************************/
/*
** Type definition (Utrx App housekeeping)
*/
typedef struct
{
    int16 TempBrd;
    int16 LastRssi;
    int16 LastRferr;
    uint8 ActiveConf;
    uint16 BootCount;
    uint32 BootCause;
    uint32 LastContact;
    uint32 TotTxBytes;
    uint32 TotRxBytes;

} UTRX_HkTlm_Payload_t;



typedef struct
{   
    uint8 ActiveConf; //0x0018
    uint16 BootCount; //0x0020
    uint32 BootCause; //0x0024
    
}__attribute__((packed)) UTRX_BcnTlm_Payload_t;

#endif
