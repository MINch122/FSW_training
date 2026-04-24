/************************************************************************
 * NASA Docket No. GRPT-18,917-1, and identified as “CFS Data Storage
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

void UT_RPT_Dispatch_MsgSizeHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_MSG_Size_t *Size            = UT_Hook_GetArgValueByName(Context, "Size", CFE_MSG_Size_t *);
    CFE_MSG_Size_t *TestCaseMsgSize = UserObj;

    *Size = *TestCaseMsgSize;
}

void UT_RPT_Dispatch_MsgIdHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_SB_MsgId_t *MsgId         = UT_Hook_GetArgValueByName(Context, "MsgId", CFE_SB_MsgId_t *);
    CFE_SB_MsgId_t *TestCaseMsgId = UserObj;

    *MsgId = *TestCaseMsgId;
}

void UT_RPT_Dispatch_FcnCodeHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_MSG_FcnCode_t *FcnCode         = UT_Hook_GetArgValueByName(Context, "FcnCode", CFE_MSG_FcnCode_t *);
    CFE_MSG_FcnCode_t *TestCaseFcnCode = UserObj;

    *FcnCode = *TestCaseFcnCode;
}

void UT_RPT_Dispatch_SetMsgSize(CFE_MSG_Size_t MsgSize)
{
    static CFE_MSG_Size_t TestCaseMsgSize;

    TestCaseMsgSize = MsgSize;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetSize), UT_RPT_Dispatch_MsgSizeHandler, &TestCaseMsgSize);
}

void UT_RPT_Dispatch_SetMsgId(CFE_SB_MsgId_t MsgId)
{
    static CFE_SB_MsgId_t TestCaseMsgId;

    TestCaseMsgId = MsgId;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetMsgId), UT_RPT_Dispatch_MsgIdHandler, &TestCaseMsgId);
}

void UT_RPT_Dispatch_SetFcnCode(CFE_MSG_FcnCode_t FcnCode)
{
    static CFE_MSG_FcnCode_t TestCaseFcnCode;

    TestCaseFcnCode = FcnCode;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetFcnCode), UT_RPT_Dispatch_FcnCodeHandler, &TestCaseFcnCode);
}

void RPT_VerifyCmdLength_Test_Nominal(void) {

    RPT_NoopCmd_t     CmdPkt;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = RPT_NOOP_CC;
    size_t            MsgSize   = sizeof(CmdPkt);

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(FcnCode);
    UT_RPT_Dispatch_SetMsgSize(MsgSize);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(RPT_VerifyCmdLength(CFE_MSG_PTR(CmdPkt.CommandHeader), sizeof(CmdPkt)));

    /* Verify results */
    UtAssert_UINT8_EQ(RPT_Data.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void RPT_VerifyCmdLength_Test_LenError(void) {

    RPT_NoopCmd_t     CmdPkt;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = RPT_NOOP_CC;
    size_t            MsgSize   = sizeof(CmdPkt);

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(FcnCode);
    UT_RPT_Dispatch_SetMsgSize(MsgSize);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(RPT_VerifyCmdLength(CFE_MSG_PTR(CmdPkt.CommandHeader), 23));

    /* Verify results */
    UtAssert_UINT8_EQ(RPT_Data.ErrCounter, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, RPT_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

// void RPT_VerifyCmdLength_Test_LenErrorNotMID(void) {

//     RPT_NoopCmd_t     CmdPkt;
//     CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
//     CFE_MSG_FcnCode_t FcnCode   = RPT_NOOP_CC;
//     size_t            MsgSize   = sizeof(CmdPkt);

//     UT_RPT_Dispatch_SetMsgId(TestMsgId);
//     UT_RPT_Dispatch_SetFcnCode(FcnCode);
//     UT_RPT_Dispatch_SetMsgSize(MsgSize);

//     /* Execute the function being tested */
//     UtAssert_BOOL_TRUE(RPT_VerifyCmdLength(CFE_MSG_PTR(CmdPkt.CommandHeader), 23));

//     /* Verify results */
//     UtAssert_UINT8_EQ(RPT_Data.ErrCounter, 1);

//     UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, RPT_CMD_LEN_ERR_EID);
//     UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
// }

void RPT_TaskPipe_Test_CmdNominal(void) {
    /**
     * Test case: RPT_CMD_MID
     */
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = RPT_NOOP_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(FcnCode);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_TaskPipe(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_NoopCmd, 1);
}

void RPT_TaskPipe_Test_SendBcnNominal(void) {
    /**
     * Test case: RPT_SEND_BCN_MID
     */
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(RPT_SEND_BCN_MID);
    CFE_MSG_FcnCode_t FcnCode   = RPT_NOOP_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(FcnCode);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_TaskPipe(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_SendBeaconCmd, 1);
}

void RPT_TaskPipe_UpdateOpsData_Nominal(void) {
    /**
     * Test case: CFE_TIME_ONEHZ_CMD_MID
     */
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(CFE_TIME_ONEHZ_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = RPT_NOOP_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(FcnCode);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_TaskPipe(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_UpdateOperationData, 1);
}

void RPT_TaskPipe_Test_MIDError(void) {
    /**
     * Test case: RPT_MID_ERR_EID
     */
    CFE_SB_MsgId_t TestMsgId = RPT_UT_MID_1;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_TaskPipe(&UT_CmdBuf.Buf));

    /* Verify Results */
    UtAssert_UINT16_EQ(RPT_Data.CmdCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, RPT_MID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

/************************************************
 * 
 * Nominal Case of Each command
 * 
 ***********************************************/
void RPT_ProcessGroundCommand_Test_NoopNominal(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_NOOP_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_NoopCmd, 1);
}

void RPT_ProcessGroundCommand_Test_ResetNominal(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_RESET_COUNTER_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_ResetCounterCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_ResetCounterCmd, 1);
}

void RPT_ProcessGroundCommand_Test_ReportNominal(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_REPORT_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_ReportCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_ReportCmd, 1);
}

void RPT_ProcessGroundCommand_Test_ClearQNominal(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_CLEAR_QUEUE_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_ClearQueueCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_ClearQueueCmd, 1);
}

/************************************************
 * 
 * Invalid Length Case of Each command
 * 
 ***********************************************/
void RPT_ProcessGroundCommand_Test_NoopInvalidLength(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_NOOP_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_NoopCmd_t)-1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_NoopCmd, 0);
}

void RPT_ProcessGroundCommand_Test_ResetInvalidLength(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_RESET_COUNTER_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_ResetCounterCmd_t)-1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_ResetCounterCmd, 0);
}

void RPT_ProcessGroundCommand_Test_ReportInvalidLength(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_REPORT_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_ReportCmd_t)-1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_ReportCmd, 0);
}

void RPT_ProcessGroundCommand_Test_ClearQInvalidLength(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = RPT_CLEAR_QUEUE_CC;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);
    UT_RPT_Dispatch_SetMsgSize(sizeof(RPT_ClearQueueCmd_t)-1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(RPT_ClearQueueCmd, 0);
}

void RPT_ProcessGroundCommand_Test_InvalidCC(void) {
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(RPT_CMD_MID);
    CFE_MSG_FcnCode_t CC = 488;

    UT_RPT_Dispatch_SetMsgId(TestMsgId);
    UT_RPT_Dispatch_SetFcnCode(CC);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(RPT_ProcessGroundCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT16_EQ(RPT_Data.CmdCounter, 0);
    UtAssert_UINT16_EQ(RPT_Data.ErrCounter, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, RPT_CC_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}


void UtTest_Setup(void){
    UT_RPT_ADD_TEST(RPT_VerifyCmdLength_Test_Nominal);
    UT_RPT_ADD_TEST(RPT_VerifyCmdLength_Test_LenError);
    UT_RPT_ADD_TEST(RPT_TaskPipe_Test_CmdNominal);
    UT_RPT_ADD_TEST(RPT_TaskPipe_Test_SendBcnNominal);
    UT_RPT_ADD_TEST(RPT_TaskPipe_UpdateOpsData_Nominal);
    UT_RPT_ADD_TEST(RPT_TaskPipe_Test_MIDError);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_NoopNominal);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_ResetNominal);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_ReportNominal);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_ClearQNominal);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_NoopInvalidLength);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_ResetInvalidLength);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_ReportInvalidLength);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_ClearQInvalidLength);
    UT_RPT_ADD_TEST(RPT_ProcessGroundCommand_Test_InvalidCC);
}