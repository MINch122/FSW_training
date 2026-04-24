/************************************************************************
 * NASA Docket No. GSC-18,917-1, and identified as “CFS Data Storage
 * (DS) application version 2.6.1”
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
 *   This file contains unit test cases for the functions contained in the file rpt_task.c
 */

#include "rpt_coveragetest_common.h"

void RPT_Util_TableInit_Test_Nominal(void) {
    /* Execute & validate the function being tested */
    UtAssert_INT32_EQ(RPT_TableInit(), CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_TBL_Register, 1);
    UtAssert_STUB_COUNT(CFE_TBL_Load, 1);
    UtAssert_STUB_COUNT(CFE_TBL_GetAddress, 1);
    UtAssert_STUB_COUNT(CFE_SB_CreatePipe, 2);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

}

void RPT_Util_TableInit_Test_TBLRegister_Error(void) {
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    UtAssert_INT32_EQ(RPT_TableInit(), -1);

    UtAssert_STUB_COUNT(CFE_TBL_Register, 1);
    UtAssert_STUB_COUNT(CFE_TBL_Load, 0);
    UtAssert_STUB_COUNT(CFE_TBL_GetAddress, 0);
    UtAssert_STUB_COUNT(CFE_SB_CreatePipe, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Util_TableInit_Test_TBLLoad_Error(void) {
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 1, -1);

    UtAssert_INT32_EQ(RPT_TableInit(), -1);

    UtAssert_STUB_COUNT(CFE_TBL_Register, 1);
    UtAssert_STUB_COUNT(CFE_TBL_Load, 1);
    UtAssert_STUB_COUNT(CFE_TBL_GetAddress, 0);
    UtAssert_STUB_COUNT(CFE_SB_CreatePipe, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Util_TableInit_Test_TBLGetAddr_Error(void) {
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    UtAssert_INT32_EQ(RPT_TableInit(), -1);

    UtAssert_STUB_COUNT(CFE_TBL_Register, 1);
    UtAssert_STUB_COUNT(CFE_TBL_Load, 1);
    UtAssert_STUB_COUNT(CFE_TBL_GetAddress, 1);
    UtAssert_STUB_COUNT(CFE_SB_CreatePipe, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Util_TableInit_Test_SBCreatePipe_Error(void) {
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    UtAssert_INT32_EQ(RPT_TableInit(), -1);

    UtAssert_STUB_COUNT(CFE_TBL_Register, 1);
    UtAssert_STUB_COUNT(CFE_TBL_Load, 1);
    UtAssert_STUB_COUNT(CFE_TBL_GetAddress, 1);
    UtAssert_STUB_COUNT(CFE_SB_CreatePipe, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);

    UT_ResetState(0);

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 2, -1);

    UtAssert_INT32_EQ(RPT_TableInit(), -1);

    UtAssert_STUB_COUNT(CFE_TBL_Register, 1);
    UtAssert_STUB_COUNT(CFE_TBL_Load, 1);
    UtAssert_STUB_COUNT(CFE_TBL_GetAddress, 1);
    UtAssert_STUB_COUNT(CFE_SB_CreatePipe, 2);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Util_OpsDataInit_Test_Nominal(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenOpsFile), 1, 1);

    UtAssert_INT32_EQ(RPT_OpsDataInit(), CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 3);
    UtAssert_STUB_COUNT(CFE_ES_GetResetType, 1);
}

void RPT_Util_OpsDataInit_Test_OpenOpsError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenOpsFile), 1, 0);

    UtAssert_INT32_EQ(RPT_OpsDataInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_UINT32_EQ(context_CFE_EVS_SendEvent[0].EventID, RPT_DATA_OPEN_ERR_EID);
}

void RPT_Util_CriticalQInit_Test_Nominal(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenCriticalFile), 1, 1);

    UtAssert_INT32_EQ(RPT_CriticalQInit(), CFE_SUCCESS);

    UtAssert_STUB_COUNT(RPT_CalculateCRC, 1);
}

void RPT_Util_CriticalQInit_Test_OpenError(void) {
    UT_SetDeferredRetcode(UT_KEY(OS_OpenCreate), 1, -1);

    UtAssert_INT32_EQ(RPT_CriticalQInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_UINT32_EQ(context_CFE_EVS_SendEvent[0].EventID, RPT_DATA_OPEN_ERR_EID);
}


void UtTest_Setup(void){
    UT_RPT_ADD_TEST(RPT_Util_TableInit_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_TableInit_Test_TBLRegister_Error);
    UT_RPT_ADD_TEST(RPT_Util_TableInit_Test_TBLLoad_Error);
    UT_RPT_ADD_TEST(RPT_Util_TableInit_Test_TBLGetAddr_Error);
    UT_RPT_ADD_TEST(RPT_Util_TableInit_Test_SBCreatePipe_Error);
    UT_RPT_ADD_TEST(RPT_Util_OpsDataInit_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_OpsDataInit_Test_OpenOpsError);
    UT_RPT_ADD_TEST(RPT_Util_CriticalQInit_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_CriticalQInit_Test_OpenError);
}