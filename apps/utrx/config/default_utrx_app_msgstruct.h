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
 *   Specification for the UTRX_APP command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in utrx_app_msgdefs.h.
 */
#ifndef UTRX_APP_MSGSTRUCT_H
#define UTRX_APP_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "utrx_app_mission_cfg.h"
#include "utrx_app_msgdefs.h"
#include "cfe_msg_hdr.h"
#include <stddef.h> /*offsetof*/
#include <string.h> /*memset*/
#include "rpt_interface_cfg.h"

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
} UTRX_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} UTRX_APP_ResetCountersCmd_t;


/*************************************************************************/
/*
** Type definition (Utrx App housekeeping)
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} UTRX_APP_SendHkCmd_t;

// typedef struct
// {
//     CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
//     UTRX_APP_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
// } UTRX_APP_HkTlm_t;

////
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    /* data */
} UTRX_NoArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 arg;

} UTRX_U8ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16 arg;

} UTRX_U16ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint32 arg;

} UTRX_U32ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint64 arg;

} UTRX_U64ArgsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;

} UTRX_APP_ResetAppCmdCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UTRX_APP_ResetDeviceCmdCountersCmd_t;


/**************SET*********************************** */

typedef UTRX_NoArgsCmd_t UTRX_AX100_GndwdtClear_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_UTRX_Reboot_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_ResetCountersCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_SetDefaultBaudCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_RparamSave1Cmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_RparamSave5Cmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_RparamSaveAllCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_CheckStatePingCmd_t;

typedef UTRX_U32ArgsCmd_t UTRX_AX100_RXCONF_SetBaudCmd_t;
typedef UTRX_U32ArgsCmd_t UTRX_AX100_TXCONF_SetBaudCmd_t;
typedef UTRX_U32ArgsCmd_t UTRX_AX100_RXCONF_SetFreqCmd_t;
typedef UTRX_U32ArgsCmd_t UTRX_AX100_TXCONF_SetFreqCmd_t;

/***********************************************************/

/*********GET********************************************/


typedef UTRX_NoArgsCmd_t UTRX_AX100_GetStatusConfigurationCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetRxBaudCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetRxGuardCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetRxFreqCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetTxBaudCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetTxFreqCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetTempBrdCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetLastRssiCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetLastRferrCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetActiveConfCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetBootCountCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetBootCauseCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetLastContactCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetTotTxBytesCmd_t;
typedef UTRX_NoArgsCmd_t UTRX_AX100_GetTotRxBytesCmd_t;


/**************Telemetry******************** */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    UTRX_HkTlm_Payload_t Payload;
} UTRX_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    UTRX_BcnTlm_Payload_t Payload;
} UTRX_BcnTlm_t;


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} UTRX_ReportTlm_t;

#endif /* UTRX_APP_MSGSTRUCT_H */
