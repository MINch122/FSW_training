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
 *   Specification for the UANT command and telemetry // sb로 command도 msg로 받지만 app이 tlm도 msg로 sb에 보낸다. 
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in UANT_msgdefs.h.
 */
#ifndef UANT_MSGSTRUCT_H
#define UANT_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "uant_mission_cfg.h"
#include "uant_msgdefs.h"
#include "cfe_msg_hdr.h"

/************************************************************************
 * UANT App Command Message Structures
 ************************************************************************/

/*
** No-Arguments Commands
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ResetAppCmdCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ResetDeviceCmdCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_DeviceInitCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ISIS_ResetCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ISIS_ReportDeploymentStatusCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ISIS_ArmAntennaSystemsCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ISIS_DisarmCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ISIS_CancelDeploymentActivationCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_ISIS_MeasureSystemTemperatureCmd_t;

/*
** 1-Byte Argument Commands
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_AutomatedDeploymentCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt1Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt2Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt3Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt4Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt1OverrideCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt2OverrideCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt3OverrideCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_DeployAnt4OverrideCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_ReportAntActivationCntCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                  Arg;
} UANT_ISIS_ReportAntActivationTimeCmd_t;



/*
** Send Housekeeping Packet Command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_SendHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_SendBcnCmd_t;
/*
** Housekeeping Telemetry Packet
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    UANT_HkTlm_Payload_t Payload;
} UANT_HkTlm_t;

/*
** Beacon Telemetry Packet
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    UANT_BcnTlm_Payload_t Payload;
} UANT_BcnTlm_t;

/*
** Operation Telemetry Packet
*/
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint16 Payload;
}UANT_GET_DEPLOYMENT_STATUS_t;

typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint8 Payload;
}UANT_REPORT_ANT_ACTIVATION_CNT_t;

typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint16 Payload;
}UANT_MEASURE_SYSTEM_TEMPERATURE_t;

typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint16 Payload;
}UANT_REPORT_ANT_ACTIVATION_TIME_t;


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Payload;
} UANT_RPT_Tlm_t;


#endif /* UANT_MSGSTRUCT_H */

