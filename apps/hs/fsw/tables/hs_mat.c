/************************************************************************
 * NASA Docket No. GSC-18,920-1, and identified as “Core Flight
 * System (cFS) Health & Safety (HS) Application version 2.4.1”
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
 *  The CFS Health and Safety (HS) Message Actions Table Definition
 */

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "hs_mission_cfg.h"
#include "hs_tbl.h"
#include "hs_tbldefs.h"
#include "cfe_tbl_filedef.h"

#include "cfe_tbl_msg.h"
#include "cfe_es_msg.h"
#include "cfe_msgids.h"

#include "ci_lab_msgids.h"
#include "ci_lab_msg.h"

#include "to_lab_msgids.h"
#include "to_lab_msg.h"

#include "eps_msgids.h"
#include "eps_msg.h"

/* Checksum for each desired command - Note that if checksum is enabled, real values (non-zero) must be input */
#ifndef CFE_TBL_NOOP_CKSUM
#define CFE_TBL_NOOP_CKSUM 0x00
#endif

#ifndef CFE_ES_NOOP_CKSUM
#define CFE_ES_NOOP_CKSUM 0x00
#endif

/* Desired Command Types. Note - HS_MAX_MSG_ACT_SIZE should be sized appropriately given desired cmd types */
typedef union
{
    CFE_TBL_NoopCmd_t cmd1;   /**< \brief Desired cmd1 type */
    CFE_ES_NoopCmd_t  cmd2;   /**< \brief Desired cmd2 type */
    CI_LAB_CreateChildTaskCmd_t cmd3;
    EPS_P31U_HardResetCmd_t cmd4;
    TO_CreateChildCmd_t cmd5;

    HS_MATMsgBuf_t    MsgBuf; /**< \brief Message Buffer for alignment */
} HS_Message;

/* MAT Table Entry Structure */
typedef struct
{
    uint16     EnableState; /**< \brief If entry contains message */
    uint16     Cooldown;    /**< \brief Maximum rate at which message can be sent */
    HS_Message HsMsg;       /**< \brief HS Message/Command Entry */
} HS_MatTableEntry_t;

/* Helper macro to get size of structure elements */
#define HS_MEMBER_SIZE(member) (sizeof(((HS_Message *)0)->member))

HS_MatTableEntry_t HS_MsgActs_Tbl[HS_MAX_MSG_ACT_TYPES] = {
    /*          EnableState               Cooldown   Message */

    /*   0 */
    {.EnableState = HS_MAT_STATE_ENABLED,
     .Cooldown    = 10,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(CFE_TBL_CMD_MID, HS_MEMBER_SIZE(cmd1), CFE_TBL_NOOP_CC, CFE_TBL_NOOP_CKSUM)}},
    /*   1 */
    {.EnableState = HS_MAT_STATE_ENABLED,
     .Cooldown    = 10,
     .HsMsg.cmd2  = {CFE_MSG_CMD_HDR_INIT(CFE_ES_CMD_MID, HS_MEMBER_SIZE(cmd2), CFE_ES_NOOP_CC, CFE_ES_NOOP_CKSUM)}},
    
    /*   2 - CI lab child restart */
    {.EnableState = HS_MAT_STATE_ENABLED,
     .Cooldown    = 1,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(CI_LAB_CMD_MID, HS_MEMBER_SIZE(cmd3), CI_LAB_CREATE_CHILD_TASK_CC, 0x34)}},
    
    /*   3 - EPS Hard reset */
    {.EnableState = HS_MAT_STATE_ENABLED,
     .Cooldown    = 10,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(EPS_CMD_MID, HS_MEMBER_SIZE(cmd4), EPS_P31U_HARD_RESET_CC, 0x41)}},
    
    /*   4  - TO child restart */
    {.EnableState = HS_MAT_STATE_ENABLED,
     .Cooldown    = 1,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(TO_LAB_CMD_MID, HS_MEMBER_SIZE(cmd5), TO_CREATE_CHILD_CC, 0x10)}},
    /*   5 */
    {.EnableState = HS_MAT_STATE_DISABLED,
     .Cooldown    = 10,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(CFE_TBL_CMD_MID, HS_MEMBER_SIZE(cmd1), CFE_TBL_NOOP_CC, CFE_TBL_NOOP_CKSUM)}},
    /*   6 */
    {.EnableState = HS_MAT_STATE_DISABLED,
     .Cooldown    = 10,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(CFE_TBL_CMD_MID, HS_MEMBER_SIZE(cmd1), CFE_TBL_NOOP_CC, CFE_TBL_NOOP_CKSUM)}},
    /*   7 */
    {.EnableState = HS_MAT_STATE_DISABLED,
     .Cooldown    = 10,
     .HsMsg.cmd1  = {CFE_MSG_CMD_HDR_INIT(CFE_TBL_CMD_MID, HS_MEMBER_SIZE(cmd1), CFE_TBL_NOOP_CC, CFE_TBL_NOOP_CKSUM)}}

};

CFE_TBL_FILEDEF(HS_MsgActs_Tbl, HS.MsgActs_Tbl, HS MsgActs Table, hs_mat.tbl)