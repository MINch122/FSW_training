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

#ifdef HAVE_FM
#include "fm_msgids.h"
#endif

#ifdef HAVE_DS
#include "ds_msgids.h"
#endif

/**
 * Include the hdr `*_msgids.h` of each app
 */
/*********************
 * FSW Header 
 ********************/
#include "rpt_msgids.h"
#include "rpt_msg.h"

#include "mission_msgids.h"
#include "mission_msg.h"
/* End of FSW Header */

/*********************
 * COMS Header 
 ********************/
#include "utrx_msgids.h"
#include "utrx_msg.h"

#include "ltrx_msgids.h"
#include "ltrx_msg.h"

/* End of COMS Header */

/*********************
 * EPS Header
 ********************/
#include "eps_msgids.h"
#include "eps_msg.h"

#include "gpio_msgids.h"
#include "gpio_msg.h"
/* End of EPS Header */

/*********************
 * SP Header
 ********************/
/* End of SP Header */

/*********************
 * ADCS Header 
 ********************/
#include "adcs_msgids.h"
#include "adcs_msg.h"
/* End of ADCS Header */

#include "ttc_msgids.h"

#include "pay_slt_msgids.h"

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

        /* Beacon requests for apps copied into HK_COMBINED_PKT1 */
        {CFE_SB_MSGID_WRAP_VALUE(RPT_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(UTRX_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(UTRX_SEND_HK_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(LTRX_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(EPS_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(GPIO_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(ADCS_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(HK_SEND_COMBINED_PKT_MID), SCH_LAB_TICK_RATE * 2, 0,
         sizeof(HK_SendCombinedPkt_Payload_t), {(uint16)HK_COMBINED_PKT1_MID, 0}},

        {CFE_SB_MSGID_WRAP_VALUE(PAY_SLT_SEND_HK_MID), SCH_LAB_TICK_RATE * 30, 0},

        {CFE_SB_MSGID_WRAP_VALUE(HK_SEND_COMBINED_PKT_MID), SCH_LAB_TICK_RATE * 10, 0,
         sizeof(HK_SendCombinedPkt_Payload_t), {(uint16)HK_COMBINED_PKT2_MID, 0}},

        {CFE_SB_MSGID_WRAP_VALUE(PAY_SLT_SEND_BCN_MID), SCH_LAB_TICK_RATE * 30, 0},
        {CFE_SB_MSGID_WRAP_VALUE(TTC_ONEHZ_WAKEUP_MID), SCH_LAB_TICK_RATE, 0},
        // /* Periodic wakeups for apps with internal timed work */
        // {CFE_SB_MSGID_WRAP_VALUE(ADCS_LOOP_MID), SCH_LAB_TICK_RATE, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(LGBAT_WAKEUP_MID), SCH_LAB_TICK_RATE * 5, 0},

        /* Example of including additional open source apps */
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_SEND_HK_MID), 100, 0}, /* Example of a 1hz packet */
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_SEND_HK_MID), 50, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_SEND_HK_MID), 98, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_SEND_HK_MID), 97, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_SEND_HK_MID), 96, 0},
        // {CFE_SB_MSGID_WRAP_VALUE(PAYUZUC_SEND_HK_MID), 1000, 0},
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
#ifdef HAVE_FM
        // {CFE_SB_MSGID_WRAP_VALUE(FM_SEND_HK_MID), 101, 0},
#endif
#ifdef HAVE_DS
        // {CFE_SB_MSGID_WRAP_VALUE(DS_SEND_HK_MID), 102, 0},
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
