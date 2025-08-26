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
 *   Solar Panel Deploy RTS
 *
 * This source file creates a RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * ------------ RTS #6 ------------
 *   LC     Halt ActionPoint to Active
 *   SP     Start Deploy Task
 * 
 * Total 2 commands
 * 
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

#include "lc_msgids.h"
#include "lc_msg.h"
#include "sp_msgids.h"
#include "sp_msg.h"

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    LC_SetAPStateCmd_t cmd1;

    SC_RtsEntryHeader_t hdr2;
    SP_StartDeployTaskCmd_t cmd2;

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
    /**
     *  1  LC Change Action point to Active (SP Halt AP)
     * */
    .rts.hdr1.WakeupCount       = 0,
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(LC_CMD_MID, SC_MEMBER_SIZE(cmd1), LC_SET_AP_STATE_CC, 0x16),
    .rts.cmd1.Payload.APNumber = 23, // TBD, SP Start Condition AP
    .rts.cmd1.Payload.NewAPState = LC_APSTATE_ACTIVE,

    /**
     *  2  SP Start Deploy task
     * */
    .rts.hdr2.WakeupCount       = 1,
    .rts.cmd2.CommandHeader = CFE_MSG_CMD_HDR_INIT(SP_CMD_MID, SC_MEMBER_SIZE(cmd2), SP_START_DEPLOY_TASK_CC, 0x06),
    
};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts006, SC.RTS_TBL006, SC Example RTS_TBL006, sc_rts006.tbl)
