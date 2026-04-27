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
 *   Specification for the PAYUEL_AOS command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in payuel_aos_msgdefs.h.
 */
#ifndef PAYUEL_AOS_MSGSTRUCT_H
#define PAYUEL_AOS_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "payuel_aos_mission_cfg.h"
#include "payuel_aos_msgdefs.h"
#include "cfe_msg_hdr.h"

/*************************************************************************/

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_AOS_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_AOS_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_AOS_ResetCmd_t;

/*
    ADS1115 Register Write& Read
        reg_addr = 0x00(conversion) or 0x01(config) or 0x02(Lo Thresh) or 0x03(Hi Thresh)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**Command header */
    uint8_t reg_addr;                       /**Register address */
    uint16_t data;                          /**Register data */
} __attribute__((packed))PAYUEL_AOS_Write_RegisterCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    uint8_t reg_addr;                       /**< \brief Register address */
} __attribute__((packed))PAYUEL_AOS_Read_RegisterCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_AOS_ReadAllChannelsCmd_t;

/*
** Type definition (Payuel Aos housekeeping)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_AOS_SendHkCmd_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    PAYUEL_AOS_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} PAYUEL_AOS_HkTlm_t;




#endif /* PAYUEL_AOS_MSGSTRUCT_H */
