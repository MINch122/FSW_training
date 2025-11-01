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
 *   CFS Stored Command (SC) RTS table 6
 *   GPS Log handle RTS
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #6 ------------
 *   GPS_OEM_CMD_LOG_ONTIME_CC
 *   GPS_OEM_LOG_HANDLER_ACTIVATE_CC
 * 
 *   ------- 10 min interval-------
 * 
 *   GPS_OEM_LOG_HANDLER_DEACTIVATE_CC
 *   FM Move file - Rename the GPS solution file "BEFORE" UZURO mission
 *   SC  Enable RTS 5
 *   SC   Start RTS 5
 * 
 * Total 6 commands
 * 
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

#include "gps_msgids.h"
#include "gps_msg.h"

#include "fm_msgids.h"
#include "fm_msg.h"

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    GPS_OEMCmd_LogOnTimeCmd_t cmd1;

    SC_RtsEntryHeader_t hdr2;
    GPS_OEMLog_HandlerActivateCmd_t cmd2;

    SC_RtsEntryHeader_t hdr3;
    GPS_OEMLog_HandlerDeactivateCmd_t cmd3;

    SC_RtsEntryHeader_t hdr4;
    FM_MoveFileCmd_t cmd4;

    SC_RtsEntryHeader_t hdr5;
    SC_EnableRtsCmd_t cmd5;

    SC_RtsEntryHeader_t hdr6;
    SC_StartRtsCmd_t cmd6;

} SC_RtsStruct006_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct006_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable006_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct006_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable006_t SC_Rts006 = {
    /* GPS solution Sequence "BEFORE" UZURO Mission */
    /**
     * 1  GPS_OEM_CMD_LOG_ONTIME_CC
     */
    .rts.hdr1.WakeupCount       = 0,
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(GPS_CMD_MID, SC_MEMBER_SIZE(cmd1), GPS_OEM_CMD_LOG_ONTIME_CC, 0xBC),
    .rts.cmd1.portIndex = 0,
    .rts.cmd1.Payload.portIndex = 0,
    .rts.cmd1.Payload.msgId = 241,
    .rts.cmd1.Payload.port = 0xC0,
    .rts.cmd1.Payload.period = 60, /* 60 sec? */
    .rts.cmd1.Payload.offset = 0.1,

    /**
     * 2  GPS_OEM_LOG_HANDLER_ACTIVATE_CC
     */
    .rts.hdr2.WakeupCount       = 1, // 0.5 sec
    .rts.cmd2.CommandHeader = CFE_MSG_CMD_HDR_INIT(GPS_CMD_MID, SC_MEMBER_SIZE(cmd2), GPS_OEM_LOG_HANDLER_ACTIVATE_CC, 0x90),
    .rts.cmd2.Payload.msgId = 241,

    /**
     * 3  GPS_OEM_LOG_HANDLER_DEACTIVATE_CC
     */
    .rts.hdr3.WakeupCount       = 2 * 60 * 10,  // 10 minuute
    .rts.cmd3.CommandHeader = CFE_MSG_CMD_HDR_INIT(GPS_CMD_MID, SC_MEMBER_SIZE(cmd3), GPS_OEM_LOG_HANDLER_DEACTIVATE_CC, 0x91),
    .rts.cmd3.Payload.msgId = 241,

    /**
     * 4  FM Move file
     */
    .rts.hdr4.WakeupCount       = 10, // 5 sec
    .rts.cmd4.CommandHeader = CFE_MSG_CMD_HDR_INIT(FM_CMD_MID, SC_MEMBER_SIZE(cmd4), FM_MOVE_FILE_CC, 0x30),
    .rts.cmd4.Payload.Source = "/cf/sdcard/bestxyz.bin",
    .rts.cmd4.Payload.Target = "/cf/sdcard/bestxyz_before.bin",
    .rts.cmd4.Payload.Overwrite = false,

    /**
     * 5  SC Enable RTS 5
     */
    .rts.hdr5.WakeupCount       = 0,  
    .rts.cmd5.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd5), SC_ENABLE_RTS_CC, 0x00),
    .rts.cmd5.Payload.RtsNum = 5,
    .rts.cmd5.Payload.Padding = 0,

    /**
     * 6  SC Start RTS 5
     */
    .rts.hdr6.WakeupCount       = 1,  // 0.5 sec
    .rts.cmd6.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd6), SC_START_RTS_CC, 0x03),
    .rts.cmd6.Payload.RtsNum = 5,
    .rts.cmd6.Payload.Padding = 0,
    
};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts006, SC.RTS_TBL006, SC Example RTS_TBL006, sc_rts006.tbl)
