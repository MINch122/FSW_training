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
 *   Specification for the PAYUEL_LGPM command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in PAYUEL_LGPM_msgdefs.h.
 */
#ifndef PAYUEL_LGPM_MSGSTRUCT_H
#define PAYUEL_LGPM_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "payuel_lgpm_mission_cfg.h"
#include "payuel_lgpm_msgdefs.h"
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
} PAYUEL_LGPM_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_LGPM_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_LGPM_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
} PAYUEL_LGPM_DisplayParamCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
} PAYUEL_LGPM_NativeCANCmd_t;


/*************************************************************************/
/*
** GroundSystem to OBC
*/

// 지상국에서 받는 명령 구조체 (총 12바이트: 헤더 8 + 시간 4)
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint32 Ground_UTC_Time;

} PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t;

/*************************************************************************/
/*
** OBC to Payload (No Args)
*/

// typedef struct
// {
//     CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
//     PAYUEL_LGPM_OBC2Payload_MCU_ALIVE_CHECK_Payload_t  payload;

// } PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_3V3_PWR_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_V28_MAIN_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_V28_SUB_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_V28_SUB_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_V12_MAIN_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */\

} PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

} PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t;

/*************************************************************************/
/*
** OBC to Payload (CMD code : 0x20)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_OBC2Payload_RWA_CONTROL_Payload_t  payload;

} PAYUEL_LGPM_RWA_CONTROL_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_OBC2Payload_RWA_PWR_ON_Payload_t  payload;

} PAYUEL_LGPM_RWA_PWR_ON_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_OBC2Payload_RWA_PWR_OFF_Payload_t  payload;

} PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_OBC2Payload_RWA_SENSE_INFO_Payload_t  payload;

} PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t;

/*************************************************************************/
/*
** Payload to OBC(CMD code : 0x23)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_MCU_ALIVE_CHECK_Payload_t  payload;

} PAYUEL_LGPM_MCU_ALIVE_CHECK_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_3V3_PWR_ON_Payload_t  payload;

} PAYUEL_LGPM_3V3_PWR_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_3V3_PWR_OFF_Payload_t  payload;

} PAYUEL_LGPM_3V3_PWR_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_ON_Payload_t  payload;

} PAYUEL_LGPM_MAIN_BOOST_SW_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_OFF_Payload_t  payload;

} PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_ON_Payload_t  payload;

} PAYUEL_LGPM_SUB_BOOST_SW_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_OFF_Payload_t  payload;

} PAYUEL_LGPM_SUB_BOOST_SW_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_V28_MAIN_ON_Payload_t  payload;

} PAYUEL_LGPM_V28_MAIN_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_V28_MAIN_OFF_Payload_t  payload;

} PAYUEL_LGPM_V28_MAIN_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_V28_SUB_ON_Payload_t  payload;

} PAYUEL_LGPM_V28_SUB_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_V28_SUB_OFF_Payload_t  payload;

} PAYUEL_LGPM_V28_SUB_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_V12_MAIN_ON_Payload_t  payload;

} PAYUEL_LGPM_V12_MAIN_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_V12_MAIN_OFF_Payload_t  payload;

} PAYUEL_LGPM_V12_MAIN_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_PWR_SENSE_INFO_Payload_t  payload;

} PAYUEL_LGPM_PWR_SENSE_INFO_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_PWR_SEQ_ON_Payload_t  payload;

} PAYUEL_LGPM_PWR_SEQ_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_PWR_SEQ_OFF_Payload_t  payload;

} PAYUEL_LGPM_PWR_SEQ_OFF_Reply_Cmd_t;




/*************************************************************************/
/*
** Payload to OBC (CMD code : 0x20)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_RWA_CONTROL_Payload_t  payload;

} PAYUEL_LGPM_RWA_CONTROL_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_RWA_PWR_ON_Payload_t  payload;

} PAYUEL_LGPM_RWA_PWR_ON_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_RWA_PWR_OFF_Payload_t  payload;

} PAYUEL_LGPM_RWA_PWR_OFF_Reply_Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    PAYUEL_LGPM_Payload2OBC_RWA_SENSE_INFO_Payload_t  payload;

} PAYUEL_LGPM_RWA_SENSE_INFO_Reply_Cmd_t;



/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
*/


typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYUEL_LGPM_SendHkCmd_t;


typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */


} PAYUEL_LGPM_BOODSSWOFFTLm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    PAYUEL_LGPM_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} PAYUEL_LGPM_HkTlm_t;

#endif /* PAYUEL_LGPM_MSGSTRUCT_H */
