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

#include "cfe_tbl_filedef.h" /* Required to obtain the CFE_TBL_FILEDEF macro definition */
#include "sch_lab_tbl.h"
#include "cfe_sb_api_typedefs.h" /* Required to use the CFE_SB_MSGID_WRAP_VALUE macro */

/* This is for the standard set of CFE core app MsgID values */
#include "cfe_msgids.h"
#include "sch_lab_interface_cfg.h"


#include "ci_lab_msgids.h"

#ifdef HAVE_TO_LAB
#include "to_lab_msgids.h"
#endif

#ifdef HAVE_SAMPLE_APP
#include "sample_app_msgids.h"
#endif

#ifdef HAVE_HS
#include "hs_msgids.h"
#endif

#ifdef HAVE_FM
#include "fm_msgids.h"
#endif

#ifdef HAVE_SC
#include "sc_msgids.h"
#endif

#ifdef HAVE_DS
#include "ds_msgids.h"
#endif

#ifdef HAVE_LC
#include "lc_msgids.h"
#endif


/**
 * Include the hdr `*_msgids.h` of each app
 */
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
#include "utrx_msgids.h"
#include "utrx_msg.h"

#include "uant_msgids.h"
#include "uant_msg.h"
/* End of COMS Header */

/*********************
 * EPS Header
 ********************/
#include "eps_msgids.h"
#include "eps_msg.h"
/* End of EPS Header */

/*********************
 * BATT Header
 ********************/
#include "batt_msgids.h"
#include "batt_msg.h"
/* End of BATT Header */

/*********************
 * SP Header
 ********************/
#include "sp_msgids.h"
#include "sp_msg.h"
/* End of SP Header */

/*********************
 * ADCS Header 
 ********************/
#include "adcs_msgids.h"
#include "adcs_msg.h"
/* End of ADCS Header */


#include "sc_msgids.h"
#include "sc_msg.h"

#include "hk_msgids.h"
#include "hk_msg.h"

/*
** SCH Lab schedule table
** When populating this table:
**  1. The entire table is processed (SCH_LAB_MAX_SCHEDULE_ENTRIES) but entries with a
**     packet rate of 0 are skipped
**  2. You can have commented out entries or entries with a packet rate of 0
**  3. If the table grows too big, increase SCH_LAB_MAX_SCHEDULE_ENTRIES
*/

SCH_LAB_ScheduleTable_t SCH_LAB_ScheduleTable = {
        /* MID, Tick, CC, PayloadSz, Param */
    .TickRate = SCH_LAB_TICK_RATE, // This TickRate value is equivalent to 1 sec. If `TickRate` is `10`, `10` tick is `1` sec 
    .Config   = {
        // {CFE_SB_MSGID_WRAP_VALUE(EPS_CMD_MID), SCH_LAB_TICK_RATE, EPS_P31U_GETHK_VI_CC},
        // {CFE_SB_MSGID_WRAP_VALUE(SC_CMD_MID), SCH_LAB_TICK_RATE * 45, SC_START_RTS_CC, sizeof(SC_RtsCmd_Payload_t), {8, 0}}, // Trigger the Bcn RTS for 45 seconds
        {CFE_SB_MSGID_WRAP_VALUE(CFE_SRL_SEND_HK_MID), SCH_LAB_TICK_RATE * 20, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(EPS_CMD_MID), SCH_LAB_TICK_RATE * 5, EPS_P31U_GETHK_VI_INTERNAL_CC},
        // {CFE_SB_MSGID_WRAP_VALUE(ADCS_LOOP_MID), SCH_LAB_TICK_RATE * 30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(RPT_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(SANT_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(STRX_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(UANT_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(UTRX_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(EPS_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(SP_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(ADCS_SEND_BCN_MID), 10*30, 0},
        /* EPS periodic HK request (every 10 sec) */
        {CFE_SB_MSGID_WRAP_VALUE(EPS_SEND_HK_MID), SCH_LAB_TICK_RATE * 10, 0},
        /* BATT periodic HK request (every 10 sec) */
        {CFE_SB_MSGID_WRAP_VALUE(BATT_SEND_HK_MID), SCH_LAB_TICK_RATE * 10, 0},
        /* SP periodic HK request (every 30 sec) */
        {CFE_SB_MSGID_WRAP_VALUE(SP_SEND_HK_MID), SCH_LAB_TICK_RATE * 30, 0},
        /* SP beacon collection (every 30 sec) */
        {CFE_SB_MSGID_WRAP_VALUE(SP_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(PAYUZUC_SEND_HK_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(PAYUZUT_SEND_BCN_MID), 10*30, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(HK_SEND_COMBINED_PKT_MID), 10*30, 0, sizeof(HK_SendCombinedPkt_Payload_t), {(uint16)HK_COMBINED_PKT1_MID, 0}},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_SEND_HK_MID), 100, 0}, /* Example of a 1hz packet */
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_SEND_HK_MID), 50, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_SEND_HK_MID), 98, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_SEND_HK_MID), 97, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_SEND_HK_MID), 96, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(PAYUZUC_SEND_HK_MID), 1000, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(SC_ONEHZ_WAKEUP_MID), SCH_LAB_TICK_RATE/2, 0}, // 0.5 sec per wakeup
        // {CFE_SB_MSGID_WRAP_VALUE(CI_LAB_WAKEUP_MID), SCH_LAB_TICK_RATE * 10, 0}, // 10 sec per wakeup

/* Example of including additional open source apps */
#ifdef HAVE_CI_LAB
        // {CFE_SB_MSGID_WRAP_VALUE(CI_LAB_SEND_HK_MID), 95, 0},
#endif
#ifdef HAVE_TO_LAB
        // {CFE_SB_MSGID_WRAP_VALUE(TO_LAB_SEND_HK_MID), 94, 0},
#endif
#ifdef HAVE_SAMPLE_APP
        // {CFE_SB_MSGID_WRAP_VALUE(SAMPLE_APP_SEND_HK_MID), 93, 0},
#endif
#ifdef HAVE_SC
        // {CFE_SB_MSGID_WRAP_VALUE(SC_SEND_HK_MID), 92, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(SC_ONEHZ_WAKEUP_MID), 91, 0},
#endif
#ifdef HAVE_HS
        {CFE_SB_MSGID_WRAP_VALUE(HS_SEND_HK_MID), 90, 0}, /* Example of a message that wouldn't be sent */
#endif
#ifdef HAVE_FM
        // {CFE_SB_MSGID_WRAP_VALUE(FM_SEND_HK_MID), 101, 0},
#endif
#ifdef HAVE_DS
        // {CFE_SB_MSGID_WRAP_VALUE(DS_SEND_HK_MID), 102, 0},
#endif
#ifdef HAVE_LC
        // {CFE_SB_MSGID_WRAP_VALUE(LC_SEND_HK_MID), 103, 0},
        {CFE_SB_MSGID_WRAP_VALUE(LC_SAMPLE_AP_MID), SCH_LAB_TICK_RATE * 5, 0, 8, {0, 175, 1}},
#endif

    }};

/*
** The macro below identifies:
**    1) the data structure type to use as the table image format
**    2) the name of the table to be placed into the cFE Table File Header
**    3) a brief description of the contents of the file image
**    4) the desired name of the table image binary file that is cFE compatible
*/
CFE_TBL_FILEDEF(SCH_LAB_ScheduleTable, SCH_LAB_APP.ScheduleTable, Schedule Lab MsgID Table, sch_lab_table.tbl)
