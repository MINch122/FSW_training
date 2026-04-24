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

/**
 * @file
 *
 * Common definitions for all RPT coverage tests
 */
#ifndef RPT_COVERAGETEST_COMMON_H
#define RPT_COVERAGETEST_COMMON_H

/*
 * Includes
 */

#include "utassert.h"
#include "uttest.h"
#include "utstubs.h"

// #include "setup.h"
// #include "eventcheck.h"

#include "cfe.h"
#include "rpt_eventids.h"
#include "rpt_task.h"
#include "rpt_dispatch.h"
#include "rpt_cmd.h"
#include "rpt_utils.h"
#include "rpt_msgids.h"
#include "rpt_msg.h"
#include "rpt_init.h"
#include "rpt_tbl.h"
#include "rpt_test_utils.h"


void RPT_Test_Setup(void) {
    static RPT_Table_t SubTbl[RPT_MAX_TBL_ENTRY];
    void *fake_ptr;
    
    UT_ResetState(0);

    memset(&RPT_Data, 0, sizeof(RPT_Data));
    memset(SubTbl, 0, sizeof(SubTbl));

    RPT_Data.SubsTblPtr = SubTbl;

    /* Register custom handlers */
    UT_SetVaHandlerFunction(UT_KEY(CFE_EVS_SendEvent), RPT_UT_Handler_CFE_EVS_SendEvent, NULL);
    UT_SetVaHandlerFunction(UT_KEY(CFE_ES_WriteToSysLog), RPT_UT_Handler_CFE_ES_WriteToSysLog, NULL);

    /* Prevent Segmentation fault in Table Subscription */
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &fake_ptr, sizeof(fake_ptr), false);
}

void RPT_Test_Teardown(void) {
    /* Do nothing */
}
/*
 * Macro to add a test case to the list of tests to execute
 */
#define UT_RPT_ADD_TEST(test) UtTest_Add(test, RPT_Test_Setup, RPT_Test_Teardown, #test)

#endif