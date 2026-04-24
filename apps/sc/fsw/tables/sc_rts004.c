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
 *   CFS Stored Command (SC) sample RTS table 4
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #4 ------------
 * PAYUZUT Thruster ON
 * PAYUZUT Thruster OFF
 * 
 * GPS_OEM_CMD_LOG_ONTIME_CC
 * GPS_OEM_LOG_HANDLER_ACTIVATE_CC
 * 
 * ------- 10 min interval-------
 * 
 * GPS_OEM_LOG_HANDLER_DEACTIVATE_CC
 * FM Move file - Rename the GPS solution file "AFTER" UZURO mission
 * EPS Set out single channel 4 off 
 * 
 * Total 7 commands
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

#include "payuzuc_msgids.h"
#include "payuzuc_msg.h"

#include "payuzut_msgids.h"
#include "payuzut_msg.h"

#include "gps_msgids.h"
#include "gps_msg.h"

#include "fm_msgids.h"
#include "fm_msg.h"

#include "eps_msgids.h"
#include "eps_msg.h"

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    PAYUZUT_ThrusterOnCmd_t cmd1;

    SC_RtsEntryHeader_t hdr2;
    PAYUZUT_ThrusterOffCmd_t cmd2;

    SC_RtsEntryHeader_t hdr3;
    GPS_OEMCmd_LogOnTimeCmd_t cmd3;

    SC_RtsEntryHeader_t hdr4;
    GPS_OEMLog_HandlerActivateCmd_t cmd4;

    SC_RtsEntryHeader_t hdr5;
    GPS_OEMLog_HandlerDeactivateCmd_t cmd5;

    SC_RtsEntryHeader_t hdr6;
    FM_MoveFileCmd_t cmd6;

    SC_RtsEntryHeader_t hdr7;
    EPS_P31U_SetOutputSingleCmd_t cmd7;

} SC_RtsStruct004_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct004_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable004_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct004_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable004_t SC_Rts004 = {
    /**
     *  1  PAYUZUT Thruster On Cmd
     * */
    .rts.hdr1.WakeupCount       = 3, // 1.5 sec
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUT_CMD_MID, SC_MEMBER_SIZE(cmd1), PAYUZUT_THRUSTER_ON_CC, 0x10),

    /**
     *  2  PAYUZUC Thruster Off Cmd
     * */
    .rts.hdr2.WakeupCount       = 18, // Should be 9 sec.
    .rts.cmd2.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUT_CMD_MID, SC_MEMBER_SIZE(cmd2), PAYUZUT_THRUSTER_OFF_CC, 0x17),


    /* GPS solution Sequence "AFTER" UZURO Mission */
    /**
     * 3  GPS_OEM_CMD_LOG_ONTIME_CC
     */
    .rts.hdr3.WakeupCount       = 0,
    .rts.cmd3.CommandHeader = CFE_MSG_CMD_HDR_INIT(GPS_CMD_MID, SC_MEMBER_SIZE(cmd3), GPS_OEM_CMD_LOG_ONTIME_CC, 0xBC),
    .rts.cmd3.portIndex = 0,
    .rts.cmd3.Payload.portIndex = 0,
    .rts.cmd3.Payload.msgId = 241,
    .rts.cmd3.Payload.port = 0xC0,
    .rts.cmd3.Payload.period = 60, /* 60 sec? */
    .rts.cmd3.Payload.offset = 0.1,

    /**
     * 4  GPS_OEM_LOG_HANDLER_ACTIVATE_CC
     */
    .rts.hdr4.WakeupCount       = 1, // 0.5 sec
    .rts.cmd4.CommandHeader = CFE_MSG_CMD_HDR_INIT(GPS_CMD_MID, SC_MEMBER_SIZE(cmd4), GPS_OEM_LOG_HANDLER_ACTIVATE_CC, 0x90),
    .rts.cmd4.Payload.msgId = 241,

    /**
     * 5  GPS_OEM_LOG_HANDLER_DEACTIVATE_CC
     */
    .rts.hdr5.WakeupCount       = 2 * 60 * 10,  // 10 minuute
    .rts.cmd5.CommandHeader = CFE_MSG_CMD_HDR_INIT(GPS_CMD_MID, SC_MEMBER_SIZE(cmd5), GPS_OEM_LOG_HANDLER_DEACTIVATE_CC, 0x91),
    .rts.cmd5.Payload.msgId = 241,


    /**
     * 6  FM Move file
     */
    .rts.hdr6.WakeupCount       = 10, // 5 sec
    .rts.cmd6.CommandHeader = CFE_MSG_CMD_HDR_INIT(FM_CMD_MID, SC_MEMBER_SIZE(cmd6), FM_MOVE_FILE_CC, 0x30),
    .rts.cmd6.Payload.Source = "/cf/sdcard/bestxyz.bin",
    .rts.cmd6.Payload.Target = "/cf/sdcard/bestxyz_after.bin",
    .rts.cmd6.Payload.Overwrite = false,

    /**
     * 7  EPS Channel 4 (GPS switch) Off
     */
    .rts.hdr7.WakeupCount       = 0,
    .rts.cmd7.CommandHeader = CFE_MSG_CMD_HDR_INIT(EPS_CMD_MID, SC_MEMBER_SIZE(cmd7), EPS_P31U_SET_OUT_SINGLE_INTERNAL_CC, 0x69),
    .rts.cmd7.Payload.channel = 4,
    .rts.cmd7.Payload.value = 0,
    .rts.cmd7.Payload.delay = 0,

    
};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts004, SC.RTS_TBL004, SC Example RTS_TBL004, sc_rts004.tbl)
