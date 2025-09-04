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
 *   CFS Stored Command (SC) RTS table 8
 *   Solar Panel Deploy Halt RTS
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #8 ------------
 *   SP     GPIO out to `LOW`
 *   LC     Deploy ActionPoint to Active
 * 
 * Total 2 commands
 * 
 */

#include "cfe.h"
#include "cfe_msgids.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

#include "hk_msg.h"
#include "hk_msgids.h"

/*********************
 * FSW Header 
 ********************/
#include "cfe_srl_msg.h"
#include "rpt_msgids.h"
#include "rpt_msg.h"
/* End of FSW Header */

/*********************
 * COMS Header 
 ********************/
#include "strx_msgids.h"
#include "strx_msg.h"

#include "utrx_msgids.h"
#include "utrx_msg.h"

#include "sant_msgids.h"
#include "sant_msg.h"

#include "uant_msgids.h"
#include "uant_msg.h"
/* End of COMS Header */

/*********************
 * EPS Header 
 ********************/
#include "eps_msgids.h"
#include "eps_msg.h"

#include "sp_msgids.h"
#include "sp_msg.h"
/* End of EPS Header */

/*********************
 * ADCS Header 
 ********************/
#include "adcs_msgids.h"
#include "adcs_msg.h"
/* End of ADCS Header */

/*********************
 * PAYLOAD Header 
 ********************/
// include PAY UEL...
#include "payuzuc_msgids.h"
#include "payuzuc_msg.h"

#include "payuzut_msgids.h"
#include "payuzut_msg.h"

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    /* 1 UTRX */
    SC_RtsEntryHeader_t hdr1;
    UTRX_SendHkCmd_t cmd1;

    /* 2 SRL */
    SC_RtsEntryHeader_t hdr2;
    CFE_SRL_SendHkCmd_t cmd2;

    /* 3 RPT */
    SC_RtsEntryHeader_t hdr3;
    RPT_NoopCmd_t cmd3;

    /* 4 SANT */
    SC_RtsEntryHeader_t hdr4;
    SANT_SendBcnCmd_t cmd4;

    /* 5 UANT */
    SC_RtsEntryHeader_t hdr5;
    UANT_SendBcnCmd_t cmd5;

    /* 6 EPS */
    SC_RtsEntryHeader_t hdr6;
    EPS_SendBcnCmd_t cmd6;

    /* 7 SP */
    SC_RtsEntryHeader_t hdr7;
    SP_SendBcnCmd_t cmd7;

    /* 8 PAYUZUC */
    SC_RtsEntryHeader_t hdr8;
    PAYUZUC_SendBcnCmd_t cmd8;

    /* 9 PAYUZUT */
    SC_RtsEntryHeader_t hdr9;
    PAYUZUT_SendBcnCmd_t cmd9;

    /* 10 ADCS */
    SC_RtsEntryHeader_t hdr10;
    ADCS_SendBcnCmd_t cmd10;

    /* 11 STRX */
    SC_RtsEntryHeader_t hdr11;
    STRX_SendHkCmd_t cmd11;

    /* 12 HK send combined packet */
    SC_RtsEntryHeader_t hdr12;
    HK_SendCombinedPktCmd_t cmd12;

} SC_RtsStruct008_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct008_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable008_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct008_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable008_t SC_Rts008 = {
    /**
     *  1  UTRX
     * */
    .rts.hdr1.WakeupCount       = 0,
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(UTRX_SEND_BCN_MID, SC_MEMBER_SIZE(cmd1), 0, 0x75),

    /**
     *  2  SRL
     * */
    .rts.hdr2.WakeupCount       = 0,
    .rts.cmd2.CommandHeader = CFE_MSG_CMD_HDR_INIT(CFE_SRL_SEND_HK_MID, SC_MEMBER_SIZE(cmd2), 0, 0x28),

    /**
     *  3 RPT
     * */
    .rts.hdr3.WakeupCount       = 0,
    .rts.cmd3.CommandHeader = CFE_MSG_CMD_HDR_INIT(RPT_SEND_BCN_MID, SC_MEMBER_SIZE(cmd3), 0, 0x00),

    /**
     *  4 SANT
     * */
    .rts.hdr4.WakeupCount       = 1, // 0.5 sec
    .rts.cmd4.CommandHeader = CFE_MSG_CMD_HDR_INIT(SANT_SEND_BCN_MID, SC_MEMBER_SIZE(cmd4), 0, 0x63),

    /**
     *  5 UANT
     * */
    .rts.hdr5.WakeupCount       = 0,
    .rts.cmd5.CommandHeader = CFE_MSG_CMD_HDR_INIT(UANT_SEND_BCN_MID, SC_MEMBER_SIZE(cmd5), 0, 0x64),

    /**
     *  6 EPS
     * */
    .rts.hdr6.WakeupCount       = 0,
    .rts.cmd6.CommandHeader = CFE_MSG_CMD_HDR_INIT(EPS_SEND_BCN_MID, SC_MEMBER_SIZE(cmd6), 0, 0x55),

    /**
     *  7 SP
     * */
    .rts.hdr7.WakeupCount       = 1, // 0.5 sec
    .rts.cmd7.CommandHeader = CFE_MSG_CMD_HDR_INIT(SP_SEND_BCN_MID, SC_MEMBER_SIZE(cmd7), 0, 0x7F),

    /**
     *  8 PAYUZUC
     * */
    .rts.hdr8.WakeupCount       = 0,
    .rts.cmd8.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUC_SEND_BCN_MID, SC_MEMBER_SIZE(cmd8), 0, 0x14),

    /**
     *  9 PAYUZUT
     * */
    .rts.hdr9.WakeupCount       = 0,
    .rts.cmd9.CommandHeader = CFE_MSG_CMD_HDR_INIT(PAYUZUT_SEND_BCN_MID, SC_MEMBER_SIZE(cmd9), 0, 0x11),

    /**
     *  10 ADCS
     * */
    .rts.hdr10.WakeupCount       = 0, //0 sec
    .rts.cmd10.CommandHeader = CFE_MSG_CMD_HDR_INIT(ADCS_SEND_BCN_MID, SC_MEMBER_SIZE(cmd10), 0, 0x41),

    /**
     *  11 STRX
     * */
    .rts.hdr11.WakeupCount       = 10, // 5 sec
    .rts.cmd11.CommandHeader = CFE_MSG_CMD_HDR_INIT(STRX_SEND_BCN_MID, SC_MEMBER_SIZE(cmd11), 0, 0x71),

    /**
     *  12 HK send combined
     * */
    .rts.hdr12.WakeupCount       = 10, // 5 sec
    .rts.cmd12.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd12), 0, 0x29),
    .rts.cmd12.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID)

};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts008, SC.RTS_TBL008, SC Example RTS_TBL008, sc_rts008.tbl)
