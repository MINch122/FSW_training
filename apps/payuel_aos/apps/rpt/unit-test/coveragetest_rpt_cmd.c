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

void RPT_Command_Noop_Test_Nominal(void) {
    /* Execute the function being tested */
    UtAssert_INT32_EQ(RPT_NoopCmd(&UT_CmdBuf.NoopCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(RPT_Data.CmdCounter == 1);
    UtAssert_BOOL_TRUE(RPT_Data.ErrCounter == 0);
}

void RPT_Command_Reset_Test_Nominal(void) {
    /* Execute the function being tested */
    UtAssert_INT32_EQ(RPT_ResetCounterCmd(&UT_CmdBuf.ResetCountersCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(RPT_Data.CmdCounter == 0);
    UtAssert_BOOL_TRUE(RPT_Data.ErrCounter == 0);
}

void RPT_Command_Report_Test_Nominal(void) {
    /* Set to critical report case */
    UT_CmdBuf.ReportCmd.Payload.IsCritical = 1;
    /* Execute the function being tested */
    UtAssert_INT32_EQ(RPT_ReportCmd(&UT_CmdBuf.ReportCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(RPT_Data.CmdCounter == 1);
    UtAssert_BOOL_TRUE(RPT_Data.ErrCounter == 0);
    UtAssert_STUB_COUNT(RPT_MultipleCritical, 1);
    UtAssert_STUB_COUNT(RPT_MultipleReport, 0);



    /* Set to general report case */
    UT_CmdBuf.ReportCmd.Payload.IsCritical = 0;
    /* Execute the function being tested */
    UtAssert_INT32_EQ(RPT_ReportCmd(&UT_CmdBuf.ReportCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(RPT_Data.CmdCounter == 2); /* Counter will be cumulated */
    UtAssert_BOOL_TRUE(RPT_Data.ErrCounter == 0);
    UtAssert_STUB_COUNT(RPT_MultipleCritical, 1);
    UtAssert_STUB_COUNT(RPT_MultipleReport, 1);
}

void RPT_Command_ClearQ_Test_Nominal(void) {
    /* Set Critical Q count to none zero for verification */
    UT_CmdBuf.ClearQCmd.Payload.IsCritical = 1;
    memset(&RPT_Data.CritQueue, 23, sizeof(RPT_Data.CritQueue));

    UtAssert_INT32_EQ(RPT_ClearQueueCmd(&UT_CmdBuf.ClearQCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(RPT_Data.CmdCounter == 1);
    UtAssert_BOOL_TRUE(RPT_Data.ErrCounter == 0);
    UtAssert_STUB_COUNT(OS_MutSemTake, 1);
    UtAssert_STUB_COUNT(OS_MutSemGive, 1);
    UtAssert_BOOL_TRUE(memcmp(&RPT_Data.CritQueue, &(RPT_CriticalQueue_t){0}, sizeof(RPT_Data.CritQueue)) == 0);


    /* Set normal Q count to none zero for verification */
    UT_CmdBuf.ClearQCmd.Payload.IsCritical = 0;
    memset(&RPT_Data.RptQueue, 4, sizeof(RPT_Data.RptQueue));

    UtAssert_INT32_EQ(RPT_ClearQueueCmd(&UT_CmdBuf.ClearQCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(RPT_Data.CmdCounter == 2);
    UtAssert_BOOL_TRUE(RPT_Data.ErrCounter == 0);
    UtAssert_STUB_COUNT(OS_MutSemTake, 2);
    UtAssert_STUB_COUNT(OS_MutSemGive, 2);
    UtAssert_BOOL_TRUE(memcmp(&RPT_Data.RptQueue, &(RPT_ReportQueue_t){0}, sizeof(RPT_Data.RptQueue)) == 0);

}

void RPT_Command_UpdateOpsData_Test_Nominal(void) {
    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_UpdateOperationData());

    /* Verify results */
    UtAssert_STUB_COUNT(OS_MutSemTake, 1);
    UtAssert_STUB_COUNT(OS_MutSemGive, 1);
    UtAssert_STUB_COUNT(RPT_CalculateCRC, 1);
    UtAssert_STUB_COUNT(RPT_WriteToFile, 1);

    UtAssert_BOOL_TRUE(RPT_Data.OpsCount == 1);
}

void RPT_Command_UpdateOpsData_Test_WriteBackupNominal(void) {
    /* Execute the function being tested */
    RPT_Data.OpsCount = RPT_OPS_STORE_BACKUP_COUNT - 1;
    UtAssert_VOIDCALL(RPT_UpdateOperationData());

    /* Verify results */
    UtAssert_STUB_COUNT(OS_MutSemTake, 1);
    UtAssert_STUB_COUNT(OS_MutSemGive, 1);
    UtAssert_STUB_COUNT(RPT_CalculateCRC, 1);
    UtAssert_STUB_COUNT(RPT_WriteToFile, 2);
    UtAssert_STUB_COUNT(RPT_OpenOpsFile, 1);
    UtAssert_STUB_COUNT(RPT_CloseFile, 1);

    UtAssert_UINT32_EQ(RPT_Data.OpsData.Sequence, 1);
    UtAssert_BOOL_TRUE(RPT_Data.OpsCount == 0);
}

void RPT_Command_Report_Test_CriticalError(void) {
    UT_CmdBuf.ReportCmd.Payload.IsCritical = 1;
    UT_SetDeferredRetcode(UT_KEY(RPT_MultipleCritical), 1, -1);

    UtAssert_INT32_EQ(RPT_ReportCmd(&UT_CmdBuf.ReportCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_UINT8_EQ(RPT_Data.CmdCounter, 1);
    UtAssert_UINT8_EQ(RPT_Data.ErrCounter, 1);
}

void RPT_Command_Report_Test_ReportError(void) {
    UT_CmdBuf.ReportCmd.Payload.IsCritical = 0;
    UT_SetDeferredRetcode(UT_KEY(RPT_MultipleReport), 1, -1);

    UtAssert_INT32_EQ(RPT_ReportCmd(&UT_CmdBuf.ReportCmd), CFE_SUCCESS);

    /* Verify results */
    UtAssert_UINT8_EQ(RPT_Data.CmdCounter, 1);
    UtAssert_UINT8_EQ(RPT_Data.ErrCounter, 1);
}


void UtTest_Setup(void) {
    UT_RPT_ADD_TEST(RPT_Command_Noop_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Command_Reset_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Command_Report_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Command_ClearQ_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Command_UpdateOpsData_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Command_UpdateOpsData_Test_WriteBackupNominal);
    UT_RPT_ADD_TEST(RPT_Command_Report_Test_CriticalError);
    UT_RPT_ADD_TEST(RPT_Command_Report_Test_ReportError);
}