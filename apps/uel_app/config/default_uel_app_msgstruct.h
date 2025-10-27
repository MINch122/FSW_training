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
 *   Specification for the UEL_APP command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in uel_app_msgdefs.h.
 */
#ifndef UEL_APP_MSGSTRUCT_H
#define UEL_APP_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "uel_app_mission_cfg.h"
#include "uel_app_msgdefs.h"
#include "cfe_msg_hdr.h"
#include <stdbool.h>
/*************************************************************************/

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
/* Basic Commands */



typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UEL_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UEL_APP_ResetCountersCmd_t;

/* Sensor Commands */
typedef struct 
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UEL_APP_GetSensDataCmd_t;

/* Camera Power Commands */
typedef struct 
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UEL_APP_SetCamPowerCmd_t;


/* Motor Commands */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                   Mode;           /* Motor mode */
    uint8                   Seconds;        /* Duration in seconds */
} UEL_APP_SetMotorMode_t;

/* Camera Shot Commands */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                   ImageSlot;      /* Image slot number */
} UEL_APP_SetCamShotCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8                   ImageSlot;      /* Image slot to query */
    uint8                   ImageNumber;
} UEL_APP_GetCamShootStatusCmd_t;

/* Camera Image Commands */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8   ImgSlot;
    uint8   ImgNumber;     /* 추가 */
    uint16  ChunkNumber;
} UEL_APP_GetCamImageCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16  CMDLength;
    uint8   CMD[UEL_APP_TERMINAL_CMD_MAX_LEN];
} UEL_APP_SetTerminalCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UEL_APP_SendBcnCmd_t;


typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 ImgSlot;
    uint8 ImgNumber;
    uint16 StartChunNumber;
    uint16 EndChunNumber;
} UEL_APP_DownloadAllCmd_t;


/************************************************************************
 * Telemetry Structures - Required for UEL_APP_Data
 ************************************************************************/

typedef struct
{
    uint8_t  PI_boot_State;       
    uint8_t  CAM_detect_State;
    uint16_t PI_boot_Count;

    int16_t  IMU_ax;
    int16_t  IMU_ay;
    int16_t  IMU_az;
    int16_t  IMU_temp;

    int16_t  ESC_Ia_mA;
    int16_t  ESC_Ib_mA;
    int16_t  ESC_Ic_mA;
    int16_t  ESC_I_rms_true_mA;
    int16_t  ESC_temp_C;

    uint16_t CAM_Shutter_Count;
} UEL_APP_bcn_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    UEL_APP_bcn_Payload_t Payload;
}UEL_APP_bcnTlm_t;


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Payload;
} UEL_APP_RPT_Report_t;


#endif /* UEL_APP_MSGSTRUCT_H */
