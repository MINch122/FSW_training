/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
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
 *   Specification for the LTRX command and telemetry message data types.
 */
#ifndef LTRX_MSGSTRUCT_H
#define LTRX_MSGSTRUCT_H

#include "ltrx_mission_cfg.h"
#include "ltrx_msgdefs.h"
#include "cfe.h"
#include <stddef.h>
#include <string.h>
#include "rpt_interface_cfg.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} LTRX_NoArgsCmd_t;

typedef LTRX_NoArgsCmd_t LTRX_NoopCmd_t;
typedef LTRX_NoArgsCmd_t LTRX_ResetCountersCmd_t;
typedef LTRX_NoArgsCmd_t LTRX_ResetAppCmdCountersCmd_t;
typedef LTRX_NoArgsCmd_t LTRX_ResetDeviceCmdCountersCmd_t;

typedef LTRX_NoArgsCmd_t LTRX_SessionStartDownlinkCmd_t; /* CC=10 */
typedef LTRX_NoArgsCmd_t LTRX_SessionAbortCmd_t;         /* CC=11 */
typedef LTRX_NoArgsCmd_t LTRX_SessionResetStateCmd_t;    /* CC=12 */
typedef LTRX_NoArgsCmd_t LTRX_QueryBeaconStatusCmd_t;    /* CC=30 */
typedef LTRX_NoArgsCmd_t LTRX_QueryGnssInfoCmd_t;        /* CC=31 */

typedef LTRX_NoArgsCmd_t LTRX_DownstreamEnableCmd_t;     /* CC=40 */
typedef LTRX_NoArgsCmd_t LTRX_DownstreamDisableCmd_t;    /* CC=41 */

typedef LTRX_NoArgsCmd_t LTRX_TestCspPingCmd_t;          /* CC=50 can test */

/* Housekeeping telemetry */
typedef struct
{
    uint8  CmdCounter;
    uint8  CmdErrCounter;
    uint8  AppErrCounter;
    uint8  Spare8;

    LTRX_GNSSInfo_Payload_t     LastGnss;         /* Type 21 */
    LTRX_BeaconStatus_Payload_t LastBeaconStatus; /* Type 23 */

    uint8  HaveGnss;
    uint8  HaveBeaconStatus;
    uint8  DownstreamEnabled;
    uint8  Reserved;
} LTRX_HkTlm_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    LTRX_HkTlm_Payload_t      Payload;
} LTRX_HkTlm_t;

/* Report telemetry */
typedef struct
{
    uint32 DeviceErrCounter;

    uint8  LastRxType;
    uint8  LastRxStatus;
    uint8  SessionState;
    uint8  Reserved0;

    uint8  HaveMsgStatus;
    uint8  HaveGnss;
    uint8  HaveBeaconStatus;
    uint8  Reserved1;

    LTRX_MessageStatus_Payload_t LastMsgStatus;
    LTRX_GNSSInfo_Payload_t      LastGnss;
    LTRX_BeaconStatus_Payload_t  LastBeaconStatus;

} LTRX_BcnTlm_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    LTRX_BcnTlm_Payload_t     Payload;
} LTRX_BcnTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Report;
} LTRX_ReportTlm_t;

#endif /* LTRX_MSGSTRUCT_H */