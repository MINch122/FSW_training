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
 *   Specification for the SANT_APP command and telemetry 
 *   message constant definitions.
 *
 *  For SANT_APP this is only the function/command code definitions
 */
#ifndef SANT_APP_MSGDEFS_H
#define SANT_APP_MSGDEFS_H

#include "common_types.h"
#include "sant_app_fcncodes.h"

/*---------------- Release / Burn status ----------------
   Maps 1‑to‑1 to gs_gssb_ant6_get_release_status() payload */
typedef struct
{
    uint8_t State;             /* Burn state of the first channel (Burning = 1, Idle = 0)  */
    uint8_t Status;            /* Release status of the first channel (Released = 1, Not released = 0) */
    uint8_t BurnTimeLeft;      /* Burn time left of the first channel [s]  */
    uint8_t BurnTries;         /* Counter of have many burns there has been attempted  */
} SANT_APP_ReleaseStatus_Payload_t;

/*---------------- Backup‑timer status ---------------
   gs_gssb_ant6_get_backup_status() */
typedef struct
{
    uint8_t  State;              /* 0‑4 per ANT‑6F spec */
    uint32_t SecondsToDeploy;
} SANT_APP_BackupStatus_Payload_t;

/*---------------- Board (MCU) status ----------------*/
typedef struct
{
    uint32_t SecondsSinceBoot;
    uint8_t  RebootCount;
} SANT_APP_BoardStatus_Payload_t;

/*---------------- Temperature -----------------------*/
typedef struct
{
    int16_t  Temperature; /* °C × 1 (LM75 style) */
} SANT_APP_Temp_Payload_t;

/*---------------- Backup‑settings readback ----------*/
typedef struct
{
    uint16_t MinutesUntilDeploy;
    uint8_t  BackupActive;
    uint8_t  MaxBurnDuration;
} SANT_APP_Settings_Payload_t;

/* -------------------SANT All Tlm Payload------------------*/
typedef struct {
    SANT_APP_ReleaseStatus_Payload_t ReleaseStatus;
    SANT_APP_BackupStatus_Payload_t BackupStatus;
    SANT_APP_BoardStatus_Payload_t BoardStatus;
    SANT_APP_Temp_Payload_t Temp;
    SANT_APP_Settings_Payload_t Settings;
} SANT_APP_OperationTlm_Payload_t;

/* ------------------Downlink Telemetry------------------- */
/**
 * Housekeeping Payload
 */
typedef struct SANT_APP_HkTlm_Payload {
    uint8_t RebootCount;
} SANT_APP_HkTlm_Payload_t;


/**
 * Beacon Payload
 */
typedef struct SANT_APP_BcnTlm_Payload {
    uint8 DeployStatus;
} SANT_APP_BcnTlm_Payload_t;

#endif
