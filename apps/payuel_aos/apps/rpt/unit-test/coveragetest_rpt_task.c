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

/*
 * Function Definitions
 */
void RPT_Main_Test_Nominal(void) {

    // CFE_SB_MsgId_t forced_MID = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    // size_t forced_size = sizeof(RPT_NoopCmd_t);

    /* Set the open function value to `1`. `0` will be considered as error */
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenOpsFile), 1, 1);
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenCriticalFile), 1, 1);

    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to prevent infinite loop of Foward message function */
    /* Force to break the loop */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 2, CFE_SB_TIME_OUT);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 3, CFE_SB_TIME_OUT);

    /* Set to prevent segmentation fault */
    // UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MID, sizeof(forced_MID), false);
    // UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_size, sizeof(forced_size), false);

    UtAssert_VOIDCALL(RPT_Main());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 0);

    /*
     * Confirm that CFE_ES_ExitApp() was called at the end of execution
     */
    // UtAssert_STUB_COUNT(CFE_ES_ExitApp, 1);
}

void RPT_Main_Test_InitError(void) {

    /* Set to satisfy condition "if (Result != CFE_SUCCESS)" immediately after call to DS_AppInitialize (which calls
     * CFE_EVS_Register) */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, CFE_EVS_INVALID_PARAMETER);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_Main());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 1);

    /*
     * This can validate that the internal "RunStatus" was
     * set to CFE_ES_RunStatus_APP_ERROR, by querying the struct directly.
     */
    UtAssert_UINT32_EQ(RPT_Data.RunStatus, CFE_ES_RunStatus_APP_ERROR);
}

void RPT_Main_Test_SBError(void) {

    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to fail condition "if (Result != CFE_SUCCESS)" immediately after call to CFE_SB_RcvMsg */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_PIPE_RD_ERR);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_Main());

    /* Verify results */
    UtAssert_UINT32_EQ(RPT_Data.RunStatus, CFE_ES_RunStatus_APP_ERROR);
}

/**
 * RPT_Init
 */
void RPT_Init_Test_Nominal(void) {
    /*
     * Test Case For:
     * CFE_Status_t RPT_Init(void)
     */
    
    /* Set the open function value to `1`. `0` will be considered as error */
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenOpsFile), 1, 1);
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenCriticalFile), 1, 1);

    /* Nominal case should return CFE_SUCCESS */
    UtAssert_INT32_EQ(RPT_Init(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Init_Test_EVSRegisterError(void) {

    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);

    /* Verify */
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 1);
}

void RPT_Init_Test_SBPipeError(void) {
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);

    /* Verify */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Init_Test_SBSubscribeError(void) {
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);

    /* Verify */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

// void RPT_Init_Test_TBLRegisterError(void) {
//     UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

//     UtAssert_INT32_EQ(RPT_Init(), -1);

//     UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
// }

// void RPT_Init_Test_TBLloadError(void) {
//     UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 1, -1);

//     UtAssert_INT32_EQ(RPT_Init(), -1);

//     UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
// }

// void RPT_Init_Test_TBLGetAddrError(void) {
//     UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

//     UtAssert_INT32_EQ(RPT_Init(), -1);

//     UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
// }

void RPT_Init_Test_OsMutSemCreateError(void) {
    UT_SetDeferredRetcode(UT_KEY(OS_MutSemCreate), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Init_Test_PriorInitError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_PriorInit), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);    

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Init_Test_OpsDataInitError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_OpsDataInit), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void RPT_Init_Test_CriticalQInitError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_CriticalQInit), 1, -1);

    UtAssert_INT32_EQ(RPT_Init(), -1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}


/**
 * RPT_OpsDataInit
 */
void RPT_OpsInit_Test_OpenError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenOpsFile), 1, OS_OBJECT_ID_UNDEFINED);

    UtAssert_INT32_EQ(RPT_OpsDataInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void RPT_OpsInit_Test_ReadError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_ReadFromFile), 1, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_INT32_EQ(RPT_OpsDataInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void RPT_OpsInit_Test_WriteError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_WriteToFile), 1, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_INT32_EQ(RPT_OpsDataInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

/**
 * RPT_CriticalQInit()
 */
void RPT_CriticalQInit_Test_OpenError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_OpenCriticalFile), 1, OS_OBJECT_ID_UNDEFINED);

    UtAssert_INT32_EQ(RPT_CriticalQInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void RPT_CriticalQInit_Test_ReadError(void) {
    UT_SetDeferredRetcode(UT_KEY(RPT_ReadFromFile), 1, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_INT32_EQ(RPT_CriticalQInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void UtTest_Setup(void) {
    UT_RPT_ADD_TEST(RPT_Main_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Main_Test_InitError);
    UT_RPT_ADD_TEST(RPT_Main_Test_SBError);
    UT_RPT_ADD_TEST(RPT_Init_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Init_Test_EVSRegisterError);
    UT_RPT_ADD_TEST(RPT_Init_Test_SBPipeError);
    UT_RPT_ADD_TEST(RPT_Init_Test_SBSubscribeError);
    // UT_RPT_ADD_TEST(RPT_Init_Test_TBLRegisterError);
    // UT_RPT_ADD_TEST(RPT_Init_Test_TBLloadError);
    // UT_RPT_ADD_TEST(RPT_Init_Test_TBLGetAddrError);
    UT_RPT_ADD_TEST(RPT_Init_Test_OsMutSemCreateError);
    UT_RPT_ADD_TEST(RPT_Init_Test_PriorInitError);
    UT_RPT_ADD_TEST(RPT_Init_Test_OpsDataInitError);
    UT_RPT_ADD_TEST(RPT_Init_Test_CriticalQInitError);

    /* These are for rpt_util test */
    // UT_RPT_ADD_TEST(RPT_OpsInit_Test_OpenError);
    // UT_RPT_ADD_TEST(RPT_OpsInit_Test_ReadError);
    // UT_RPT_ADD_TEST(RPT_OpsInit_Test_WriteError);
    // UT_RPT_ADD_TEST(RPT_CriticalQInit_Test_OpenError);
    // UT_RPT_ADD_TEST(RPT_CriticalQInit_Test_ReadError);
}