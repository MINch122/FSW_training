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
#include "utstub-helpers.h"

void RPT_Util_Subscribe_Test_Nominal(void) {
    RPT_Table_t *before, *after;

    /* Force to 1st entry is being subscribed */
    RPT_Data.SubsTblPtr->UsedState = RPT_ENABLED;
    RPT_Data.SubsTblPtr->Entry.MessageID = CFE_SB_ValueToMsgId(RPT_CMD_MID);

    before = RPT_Data.SubsTblPtr;
    UtAssert_VOIDCALL(RPT_Subscribe());
    after = RPT_Data.SubsTblPtr;

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_SB_SubscribeEx, 1);
    UtAssert_BOOL_TRUE(((uintptr_t)after - (uintptr_t)before) == RPT_MAX_TBL_ENTRY * sizeof(RPT_Table_t));
}

void RPT_Util_Enqueue_Test_Nominal(void) {
    RPT_Report_t Report;
    Report.MsgID = 0x2323;
    Report.CommandCode = 23;
    RPT_Data.OpsData.TimeSec = 123;
    RPT_Data.OpsData.TimeSubsec = 456;

    UtAssert_VOIDCALL(RPT_Enqueue(&Report, 1));

    UtAssert_UINT8_EQ(RPT_Data.CritQueue.Count, 1);
    UtAssert_UINT8_EQ(RPT_Data.CritQueue.Head, 1);
    UtAssert_UINT32_EQ(RPT_Data.CritQueue.Entry[0].Time.Seconds, 123);
    UtAssert_UINT32_EQ(RPT_Data.CritQueue.Entry[0].Time.Subseconds, 456);

    UtAssert_VOIDCALL(RPT_Enqueue(&Report, 0));

    UtAssert_UINT8_EQ(RPT_Data.RptQueue.Count, 1);
    UtAssert_UINT8_EQ(RPT_Data.RptQueue.Head, 1);
}

void RPT_Util_Report_Test_Nominal(void) {
    RPT_Report_t Report;
    Report.MsgID = 0x2323;
    Report.CommandCode = 23;

    UtAssert_INT32_EQ(RPT_Report(&Report, 1), CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_MSG_Init, 1);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    UtAssert_INT32_EQ(RPT_Report(&Report, 0), CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_MSG_Init, 2);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 2);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 2);
}

void RPT_Util_Report_Test_MsgInitError(void) {
    RPT_Report_t Report;
    Report.MsgID = 0x2323;
    Report.CommandCode = 23;
    UT_SetDefaultReturnValue(UT_KEY(CFE_MSG_Init), -1);

    /* Critical Report */
    UtAssert_INT32_EQ(RPT_Report(&Report, 1), -1);

    UtAssert_STUB_COUNT(CFE_MSG_Init, 1);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);

    /* General Report */
    UtAssert_INT32_EQ(RPT_Report(&Report, 0), -1);

    UtAssert_STUB_COUNT(CFE_MSG_Init, 2);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
}

void RPT_Util_Report_Test_TransmitMsgError(void) {
    RPT_Report_t Report;
    Report.MsgID = 0x2323;
    Report.CommandCode = 23;

    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_TransmitMsg), -1);

    /* Critical Report */
    UtAssert_INT32_EQ(RPT_Report(&Report, 1), -1);

    UtAssert_STUB_COUNT(CFE_MSG_Init, 1);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    /* General Report */
    UtAssert_INT32_EQ(RPT_Report(&Report, 0), -1);

    UtAssert_STUB_COUNT(CFE_MSG_Init, 2);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 2);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 2);
}

void RPT_Util_VerifyReportLength_Test_Nominal(void) {
    size_t ActualSize = sizeof(RPT_ReportTlm_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ActualSize, sizeof(size_t), false);

    UtAssert_BOOL_TRUE(RPT_VerifyReportLength(&UT_CmdBuf.Buf.Msg) == true);
    UtAssert_STUB_COUNT(CFE_MSG_GetSize, 1);

}

void RPT_Util_VerifyReportLength_Test_LenError(void) {
    size_t ActualSize = sizeof(RPT_ReportTlm_t) - 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ActualSize, sizeof(size_t), false);

    UtAssert_BOOL_TRUE(RPT_VerifyReportLength(&UT_CmdBuf.Buf.Msg) == false);
    UtAssert_STUB_COUNT(CFE_MSG_GetSize, 1);

}

void RPT_Util_OpenOpsFile_Test_Nominal(void) {
    UtAssert_True(OS_ObjectIdDefined(RPT_OpenOpsFile()), "RPT_OpenOpsFile return Valid id.");
    UtAssert_STUB_COUNT(OS_OpenCreate, 1);
}

void RPT_UtilOpenOpsFile_Test_OpenError(void) {
    UT_SetDeferredRetcode(UT_KEY(OS_OpenCreate), 1, -1);

    UtAssert_UINT32_EQ(RPT_OpenOpsFile(), OS_OBJECT_ID_UNDEFINED);
    UtAssert_STUB_COUNT(OS_OpenCreate, 1);
}

void RPT_Util_OpenOpsBackupFile_Test_Nominal(void) {
    UtAssert_True(OS_ObjectIdDefined(RPT_OpenOpsBackupFile(23)),
                  "RPT_OpenOpsBackupFile return Valid id.");
    UtAssert_STUB_COUNT(OS_DirectoryOpen, 1);
    UtAssert_STUB_COUNT(OS_DirectoryClose, 1);
    UtAssert_STUB_COUNT(OS_mkdir, 0);
    UtAssert_STUB_COUNT(OS_OpenCreate, 1);
}

void RPT_Util_OpenOpsBackupFile_Test_CreateDirectory(void) {
    UT_SetDeferredRetcode(UT_KEY(OS_DirectoryOpen), 1, OS_ERROR);

    UtAssert_True(OS_ObjectIdDefined(RPT_OpenOpsBackupFile(23)),
                  "RPT_OpenOpsBackupFile return Valid id after mkdir.");
    UtAssert_STUB_COUNT(OS_DirectoryOpen, 1);
    UtAssert_STUB_COUNT(OS_DirectoryClose, 0);
    UtAssert_STUB_COUNT(OS_mkdir, 1);
    UtAssert_STUB_COUNT(OS_OpenCreate, 1);
}

void RPT_Util_WriteToFile_Test_Nominal(void) {
    char data[23] = {0,};
    size_t WriteSize = 23;
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, WriteSize);

    UtAssert_INT32_EQ(RPT_WriteToFile(23, data, WriteSize), CFE_SUCCESS);
    UtAssert_STUB_COUNT(OS_lseek, 1);
    UtAssert_STUB_COUNT(OS_write, 1);
}

void RPT_Util_WriteToFile_Test_WriteError(void) {
    char data[23] = {0,};
    size_t WriteSize = 27;
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, 23);

    UtAssert_INT32_EQ(RPT_WriteToFile(23, data, WriteSize), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
    UtAssert_STUB_COUNT(OS_lseek, 1);
    UtAssert_STUB_COUNT(OS_write, 1);
}

void RPT_Util_ReadFromFile_Test_Nominal(void) {
    size_t ReadSize = 23;
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, ReadSize);

    UtAssert_INT32_EQ(RPT_ReadFromFile(1, &UT_CmdBuf.Buf, ReadSize), ReadSize);
    UtAssert_STUB_COUNT(OS_read, 1);
}

void RPT_Util_ReadFromFile_Test_ReadError(void) {
    size_t ReadSize = 27;
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, OS_INVALID_POINTER);

    UtAssert_INT32_EQ(RPT_ReadFromFile(1, &UT_CmdBuf.Buf, ReadSize), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
    UtAssert_STUB_COUNT(OS_read, 1);
}

void RPT_Util_CloseFile_Test_Nominal(void) {
    osal_id_t FakeId = UT_AllocStubObjId(OS_OBJECT_TYPE_OS_STREAM);

    UtAssert_INT32_EQ(RPT_CloseFile(FakeId), CFE_SUCCESS);
    UtAssert_STUB_COUNT(OS_close, 1);
}

void RPT_Util_CloseFile_Test_CloseError(void) {
    UT_SetDeferredRetcode(UT_KEY(OS_close), 1, OS_ERROR);

    UtAssert_INT32_EQ(RPT_CloseFile(1), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
    UtAssert_STUB_COUNT(OS_close, 1);
}

void UtTest_Setup(void) {
    UT_RPT_ADD_TEST(RPT_Util_Subscribe_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_Enqueue_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_Report_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_Report_Test_MsgInitError);
    UT_RPT_ADD_TEST(RPT_Util_Report_Test_TransmitMsgError);
    UT_RPT_ADD_TEST(RPT_Util_VerifyReportLength_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_VerifyReportLength_Test_LenError);
    UT_RPT_ADD_TEST(RPT_Util_OpenOpsFile_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_UtilOpenOpsFile_Test_OpenError);
    UT_RPT_ADD_TEST(RPT_Util_OpenOpsBackupFile_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_OpenOpsBackupFile_Test_CreateDirectory);
    UT_RPT_ADD_TEST(RPT_Util_WriteToFile_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_WriteToFile_Test_WriteError);
    UT_RPT_ADD_TEST(RPT_Util_ReadFromFile_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_ReadFromFile_Test_ReadError);
    UT_RPT_ADD_TEST(RPT_Util_CloseFile_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_Util_CloseFile_Test_CloseError);
}
