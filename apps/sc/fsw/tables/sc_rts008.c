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
 *   Beacon collection and Send combined beacon packet
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #8 ------------
 * 
 * 
 * Total 12 commands
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

    /**
     * Send Combined packet
     * Beacon pattern should be "COS" in morse code
     * Interval between each combined packet treated as 'dot' or 'dash'
     * 'dot'  : interval 0.5 second
     * 'dash' : interval 1 second
     * Therefore, total 13 packet should be transmitted
     */

    /*---------------Start "C"---------------*/
    /* 12 HK send combined packet */
    SC_RtsEntryHeader_t hdr12;
    HK_SendCombinedPktCmd_t cmd12;
    /* dash */
    /* 13 HK send combined packet */
    SC_RtsEntryHeader_t hdr13;
    HK_SendCombinedPktCmd_t cmd13;
    /* dot */
    /* 14 HK send combined packet */
    SC_RtsEntryHeader_t hdr14;
    HK_SendCombinedPktCmd_t cmd14;
    /* dash */
    /* 15 HK send combined packet */
    SC_RtsEntryHeader_t hdr15;
    HK_SendCombinedPktCmd_t cmd15;
    /* dot */
    /* 16 HK send combined packet */
    SC_RtsEntryHeader_t hdr16;
    HK_SendCombinedPktCmd_t cmd16;
    /*---------------End "C"---------------*/

    /* 3 sec interval */

    /*---------------Start "O"---------------*/
    /* 17 HK send combined packet */
    SC_RtsEntryHeader_t hdr17;
    HK_SendCombinedPktCmd_t cmd17;
    /* dash */
    /* 18 HK send combined packet */
    SC_RtsEntryHeader_t hdr18;
    HK_SendCombinedPktCmd_t cmd18;
    /* dash */
    /* 19 HK send combined packet */
    SC_RtsEntryHeader_t hdr19;
    HK_SendCombinedPktCmd_t cmd19;
    /* dash */
    /* 20 HK send combined packet */
    SC_RtsEntryHeader_t hdr20;
    HK_SendCombinedPktCmd_t cmd20;
    /*---------------End "O"---------------*/

    /* 3 sec interval */

    /*---------------Start "S"---------------*/
    /* 21 HK send combined packet */
    SC_RtsEntryHeader_t hdr21;
    HK_SendCombinedPktCmd_t cmd21;
    /* dot */
    /* 22 HK send combined packet */
    SC_RtsEntryHeader_t hdr22;
    HK_SendCombinedPktCmd_t cmd22;
    /* dot */
    /* 23 HK send combined packet */
    SC_RtsEntryHeader_t hdr23;
    HK_SendCombinedPktCmd_t cmd23;
    /* dot */
    /* 24 HK send combined packet */
    SC_RtsEntryHeader_t hdr24;
    HK_SendCombinedPktCmd_t cmd24;
    /*---------------End "S"---------------*/

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
    .rts.hdr10.WakeupCount       = 0,
    .rts.cmd10.CommandHeader = CFE_MSG_CMD_HDR_INIT(ADCS_SEND_BCN_MID, SC_MEMBER_SIZE(cmd10), 0, 0x41),

    /**
     *  11 STRX
     * */
    .rts.hdr11.WakeupCount       = 1, // 0.5 sec
    .rts.cmd11.CommandHeader = CFE_MSG_CMD_HDR_INIT(STRX_SEND_BCN_MID, SC_MEMBER_SIZE(cmd11), 0, 0x71),

    /**
     *  12 HK send combined
     * */
    .rts.hdr12.WakeupCount       = 2, // 1 sec
    .rts.cmd12.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd12), 0, 0x29),
    .rts.cmd12.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dash */
    /**
     *  13 HK send combined
     * */
    .rts.hdr13.WakeupCount       = 2, // 1 sec
    .rts.cmd13.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd13), 0, 0x29),
    .rts.cmd13.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dot */
    /**
     *  14 HK send combined
     * */
    .rts.hdr14.WakeupCount       = 1, // 0.5 sec
    .rts.cmd14.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd14), 0, 0x29),
    .rts.cmd14.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dash */
    /**
     *  15 HK send combined
     * */
    .rts.hdr15.WakeupCount       = 2, // 1 sec
    .rts.cmd15.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd15), 0, 0x29),
    .rts.cmd15.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dot */
    /**
     *  16 HK send combined
     * */
    .rts.hdr16.WakeupCount       = 1, // 0.5 sec
    .rts.cmd16.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd16), 0, 0x29),
    .rts.cmd16.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),

    /* 3 sec */

    /**
     *  17 HK send combined
     * */
    .rts.hdr17.WakeupCount       = 6, // 3 sec
    .rts.cmd17.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd17), 0, 0x29),
    .rts.cmd17.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dash */
    /**
     *  18 HK send combined
     * */
    .rts.hdr18.WakeupCount       = 2, // 1 sec
    .rts.cmd18.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd18), 0, 0x29),
    .rts.cmd18.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dash */
    /**
     *  19 HK send combined
     * */
    .rts.hdr19.WakeupCount       = 2, // 1 sec
    .rts.cmd19.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd19), 0, 0x29),
    .rts.cmd19.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dash */
    /**
     *  20 HK send combined
     * */
    .rts.hdr20.WakeupCount       = 2, // 1 sec
    .rts.cmd20.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd20), 0, 0x29),
    .rts.cmd20.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    
    /* 3 sec */

    /**
     *  21 HK send combined
     * */
    .rts.hdr21.WakeupCount       = 1, // 0.5 sec
    .rts.cmd21.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd21), 0, 0x29),
    .rts.cmd21.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dot */
    /**
     *  22 HK send combined
     * */
    .rts.hdr22.WakeupCount       = 1, // 0.5 sec
    .rts.cmd22.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd22), 0, 0x29),
    .rts.cmd22.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dot */
    /**
     *  23 HK send combined
     * */
    .rts.hdr23.WakeupCount       = 1, // 0.5 sec
    .rts.cmd23.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd23), 0, 0x29),
    .rts.cmd23.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
    /* dot */
    /**
     *  24 HK send combined
     * */
    .rts.hdr24.WakeupCount       = 1, // 0.5 sec
    .rts.cmd24.CommandHeader = CFE_MSG_CMD_HDR_INIT(HK_SEND_COMBINED_PKT_MID, SC_MEMBER_SIZE(cmd24), 0, 0x29),
    .rts.cmd24.Payload.OutMsgToSend = CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID)

};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts008, SC.RTS_TBL008, SC Example RTS_TBL008, sc_rts008.tbl)
