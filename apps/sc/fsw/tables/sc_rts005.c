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
 *   CFS Stored Command (SC) sample RTS table 5
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #5 ------------
 * PAYUZUT  Cumulate Temperature cmd
 *   SC     Enable RTS #3 (KissCAM)
 *   SC     Enable RTS #4 (Thruster)
 *   SC     Start RTS #3 (KissCAM)
 *   SC     Start RTS #4 (Thruster)
 * 
 * Total 5 commands
 * 
 * --------------------------To Do---------------------------
 * This RTS should include the 5V power on to UZURO IF boards
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

#include "payuzut_msgids.h"
#include "payuzut_msg.h"

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    PAYUZUT_CumulateTempCmd_t cmd1;

    SC_RtsEntryHeader_t hdr2;
    SC_EnableRtsCmd_t cmd2;

    SC_RtsEntryHeader_t hdr3;
    SC_EnableRtsCmd_t cmd3;

    SC_RtsEntryHeader_t hdr4;
    SC_StartRtsCmd_t cmd4;

    SC_RtsEntryHeader_t hdr5;
    SC_StartRtsCmd_t cmd5;

} SC_RtsStruct005_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct005_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable005_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct005_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable005_t SC_Rts005 = {
    /**
     *  1  PAYUZUT Cumulate Temperature Cmd
     * */
    .rts.hdr1.WakeupCount       = 0,
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUT_CMD_MID, SC_MEMBER_SIZE(cmd1), PAYUZUT_CUMULATE_TEMP_CC, 0x16),

    /**
     *  2  SC Enable RTS 3 (KissCAM)
     * */
    .rts.hdr2.WakeupCount       = 1,
    .rts.cmd2.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd2), SC_ENABLE_RTS_CC, 0x06),
    .rts.cmd2.Payload.RtsNum = 3,

    /**
     *  3  SC Enable RTS 4 (Thruster)
     * */
    .rts.hdr3.WakeupCount       = 1,
    .rts.cmd3.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd3), SC_ENABLE_RTS_CC, 0x01),
    .rts.cmd3.Payload.RtsNum = 4,

    /**
     *  4  SC Start RTS 4 (Thruster)
     * */
    .rts.hdr4.WakeupCount       = 10,
    .rts.cmd4.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd4), SC_START_RTS_CC, 0x02),
    .rts.cmd4.Payload.RtsNum = 4,


    /**
     *  5  SC Start RTS 3 (KissCAM)
     * */
    .rts.hdr5.WakeupCount       = 0,
    .rts.cmd5.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd5), SC_START_RTS_CC, 0x05),
    .rts.cmd5.Payload.RtsNum = 3
    
};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts005, SC.RTS_TBL005, SC Example RTS_TBL005, sc_rts005.tbl)
