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
 *   CFS Stored Command (SC) sample RTS table 3
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #3 ------------
 * PAYUZUC set mode to SD mode (mode 1)
 * PAYUZUC capture in Memory Slot 0
 * PAYUZUC capture in Memory Slot 1
 * PAYUZUC capture in Memory Slot 2
 * PAYUZUC capture in Memory Slot 3
 * 
 * PAYUZUC  Mosaic to Memory Slot 4
 * 
 * PAYUZUC capture in Memory Slot 0
 * PAYUZUC capture in Memory Slot 1
 * PAYUZUC capture in Memory Slot 2
 * PAYUZUC capture in Memory Slot 3

 * PAYUZUC  Mosaic to Memory Slot 5
 * 
 * PAYUZUC capture in Memory Slot 0
 * PAYUZUC capture in Memory Slot 1
 * PAYUZUC capture in Memory Slot 2
 * PAYUZUC capture in Memory Slot 3
 * 
 * Total 15 commands 1 + 5 + 5 + 4
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

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    PAYUZUC_SetModeCmd_t cmd1;

    SC_RtsEntryHeader_t hdr2;
    PAYUZUC_CaptureCmd_t cmd2;

    SC_RtsEntryHeader_t hdr3;
    PAYUZUC_CaptureCmd_t cmd3;

    SC_RtsEntryHeader_t hdr4;
    PAYUZUC_CaptureCmd_t cmd4;

    SC_RtsEntryHeader_t hdr5;
    PAYUZUC_CaptureCmd_t cmd5;

    SC_RtsEntryHeader_t hdr6;
    PAYUZUC_MosaicCmd_t cmd6;



    SC_RtsEntryHeader_t hdr7;
    PAYUZUC_CaptureCmd_t cmd7;

    SC_RtsEntryHeader_t hdr8;
    PAYUZUC_CaptureCmd_t cmd8;

    SC_RtsEntryHeader_t hdr9;
    PAYUZUC_CaptureCmd_t cmd9;

    SC_RtsEntryHeader_t hdr10;
    PAYUZUC_CaptureCmd_t cmd10;

    SC_RtsEntryHeader_t hdr11;
    PAYUZUC_MosaicCmd_t cmd11;



    SC_RtsEntryHeader_t hdr12;
    PAYUZUC_CaptureCmd_t cmd12;

    SC_RtsEntryHeader_t hdr13;
    PAYUZUC_CaptureCmd_t cmd13;

    SC_RtsEntryHeader_t hdr14;
    PAYUZUC_CaptureCmd_t cmd14;

    SC_RtsEntryHeader_t hdr15;
    PAYUZUC_CaptureCmd_t cmd15;

} SC_RtsStruct003_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct003_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable003_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct003_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable003_t SC_Rts003 = {
    /**
     *  1  PAYUZUC SET MODE CMD 
     * */
    .rts.hdr1.WakeupCount       = 0,
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd1), PAYUZUC_SET_MODE_CC, 0x17),
    .rts.cmd1.Payload.MD        = (uint8)1,

    /**
     *  2  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr2.WakeupCount       = 2,
    .rts.cmd2.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd2), PAYUZUC_CAPTURE_CC, 0x12),
    .rts.cmd2.Payload.MEM       = (uint8)0,
    .rts.cmd2.Payload.TST       = (uint8)0,

    /**
     *  3  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr3.WakeupCount       = 2,
    .rts.cmd3.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd3), PAYUZUC_CAPTURE_CC, 0x13),
    .rts.cmd3.Payload.MEM       = (uint8)1,
    .rts.cmd3.Payload.TST       = (uint8)0,
    
    /**
     *  4  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr4.WakeupCount       = 2,
    .rts.cmd4.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd4), PAYUZUC_CAPTURE_CC, 0x10),
    .rts.cmd4.Payload.MEM       = (uint8)2,
    .rts.cmd4.Payload.TST       = (uint8)0,
    
    /**
     *  5  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr5.WakeupCount       = 2,
    .rts.cmd5.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd5), PAYUZUC_CAPTURE_CC, 0x11),
    .rts.cmd5.Payload.MEM       = (uint8)3,
    .rts.cmd5.Payload.TST       = (uint8)0,
    
    /**
     *  6  PAYUZUC MOSAIC CMD
     * */
    .rts.hdr6.WakeupCount       = 1,
    .rts.cmd6.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd6), PAYUZUC_MOSAIC_CC, 0x1A),
    .rts.cmd6.Payload.MEM       = (uint8)4,



    /**
     *  7  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr7.WakeupCount       = 1,
    .rts.cmd7.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd7), PAYUZUC_CAPTURE_CC, 0x12),
    .rts.cmd7.Payload.MEM       = (uint8)0,
    .rts.cmd7.Payload.TST       = (uint8)0,

    /**
     *  8  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr8.WakeupCount       = 2,
    .rts.cmd8.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd8), PAYUZUC_CAPTURE_CC, 0x13),
    .rts.cmd8.Payload.MEM       = (uint8)1,
    .rts.cmd8.Payload.TST       = (uint8)0,
    
    /**
     *  9  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr9.WakeupCount       = 2,
    .rts.cmd9.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd9), PAYUZUC_CAPTURE_CC, 0x10),
    .rts.cmd9.Payload.MEM       = (uint8)2,
    .rts.cmd9.Payload.TST       = (uint8)0,
    
    /**
     *  10  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr10.WakeupCount       = 2,
    .rts.cmd10.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd10), PAYUZUC_CAPTURE_CC, 0x11),
    .rts.cmd10.Payload.MEM       = (uint8)3,
    .rts.cmd10.Payload.TST       = (uint8)0,
    
    /**
     *  11  PAYUZUC MOSAIC CMD
     * */
    .rts.hdr11.WakeupCount       = 1,
    .rts.cmd11.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd11), PAYUZUC_MOSAIC_CC, 0x1B),
    .rts.cmd11.Payload.MEM       = (uint8)5,




    /**
     *  12  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr12.WakeupCount       = 1,
    .rts.cmd12.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd12), PAYUZUC_CAPTURE_CC, 0x12),
    .rts.cmd12.Payload.MEM       = (uint8)0,
    .rts.cmd12.Payload.TST       = (uint8)0,

    /**
     *  13  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr13.WakeupCount       = 2,
    .rts.cmd13.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd13), PAYUZUC_CAPTURE_CC, 0x13),
    .rts.cmd13.Payload.MEM       = (uint8)1,
    .rts.cmd13.Payload.TST       = (uint8)0,
    
    /**
     *  14  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr14.WakeupCount       = 2,
    .rts.cmd14.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd14), PAYUZUC_CAPTURE_CC, 0x10),
    .rts.cmd14.Payload.MEM       = (uint8)2,
    .rts.cmd14.Payload.TST       = (uint8)0,
    
    /**
     *  15  PAYUZUC CAPTURE CMD 
     * */
    .rts.hdr15.WakeupCount       = 2,
    .rts.cmd15.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_CMD_MID, SC_MEMBER_SIZE(cmd15), PAYUZUC_CAPTURE_CC, 0x11),
    .rts.cmd15.Payload.MEM       = (uint8)3,
    .rts.cmd15.Payload.TST       = (uint8)0,
    
};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts003, SC.RTS_TBL003, SC Example RTS_TBL003, sc_rts003.tbl)
