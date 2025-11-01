/************************************************************************
 * NASA Docket No. GSC-18,924-1, and identified as “Core Flight
 * System (cFS) Stored Command Application version 3.1.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
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
 *   CFS Stored Command (SC) sample RTS table 1
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * Comprehensive S/C startup Procedure
 * ------------ RTS #1 ------------
 * ADCS Boot High
 * ADCS Boot Low
 * ADCS Enable High
 * TO   Enable Tlm
 * RPT  Get operation data - EO will ingest this and determine the boot count (First deploy or not)
 * SC   Enable RTS : RTS8 (i.e. beacon sequence) is always be executed. Should Never be stopped !!!
 * 
 * Total 4 commands
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

#include "adcs_msgids.h"
#include "adcs_msg.h"

#include "to_lab_msgids.h"
#include "to_lab_msg.h"

#include "rpt_msgids.h"
#include "rpt_msg.h"

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    ADCS_GpioBootHighCmd_t cmd1;

    SC_RtsEntryHeader_t hdr2;
    ADCS_GpioBootLowCmd_t cmd2;

    SC_RtsEntryHeader_t hdr3;
    ADCS_GpioEnHighCmd_t cmd3;

    SC_RtsEntryHeader_t hdr4;
    TO_LAB_EnableOutputCmd_t cmd4;

    SC_RtsEntryHeader_t hdr5;
    RPT_GetOpsDataCmd_t cmd5;

    SC_RtsEntryHeader_t hdr6;
    SC_EnableRtsCmd_t cmd6;

} SC_RtsStruct001_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct001_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable001_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct001_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable001_t SC_Rts001 = {
    /* 1 ADCS Boot High */
    .rts.hdr1.WakeupCount = 0,
    .rts.cmd1.CommandHeader =
        CFE_MSG_CMD_HDR_INIT(ADCS_CMD_MID, SC_MEMBER_SIZE(cmd1), ADCS_GPIO_BOOT_HIGH_CC, 0x4B),

    /* 2 ADCS Boot Low */
    .rts.hdr2.WakeupCount = 24, // 12 sec
    .rts.cmd2.CommandHeader =
        CFE_MSG_CMD_HDR_INIT(ADCS_CMD_MID, SC_MEMBER_SIZE(cmd2), ADCS_GPIO_BOOT_LOW_CC, 0x4A),

    /* 3 ADCS Enable High */
    .rts.hdr3.WakeupCount = 6, // 3 sec
    .rts.cmd3.CommandHeader =
        CFE_MSG_CMD_HDR_INIT(ADCS_CMD_MID, SC_MEMBER_SIZE(cmd3), ADCS_GPIO_ENABLE_HIGH_CC, 0x46),

    /* 4 To Enable Output */
    .rts.hdr4.WakeupCount = 1, // 0.5 sec
    .rts.cmd4.CommandHeader = 
        CFE_MSG_CMD_HDR_INIT(TO_LAB_CMD_MID, SC_MEMBER_SIZE(cmd4), TO_LAB_OUTPUT_ENABLE_CC, 0x23),

    /* 5 Rpt Get ops data cmd - EO will ingest this msg, and start child task */
    .rts.hdr5.WakeupCount = 0,
    .rts.cmd5.CommandHeader =
        CFE_MSG_CMD_HDR_INIT(RPT_CMD_MID, SC_MEMBER_SIZE(cmd5), RPT_GET_OPS_DATA_CC, 0x23),

    /* 6 Enable RTS 8 */
    .rts.hdr6.WakeupCount = 0,
    .rts.cmd6.CommandHeader = 
        CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd6), SC_ENABLE_RTS_CC, 0x8E),
    .rts.cmd6.Payload.RtsNum = 8,
    .rts.cmd6.Payload.Padding = 0

};
/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts001, SC.RTS_TBL001, SC Example RTS_TBL001, sc_rts001.tbl)
