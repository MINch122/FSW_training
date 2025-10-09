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

/*
** File:
**    srl_UT.c
**
** Purpose:
**    Serial Services unit test
**
** References:
**    1. cFE Application Developers Guide
**    2. unit test standard 092503
**    3. C Coding Standard 102904
**
** Notes:
**    1. This is unit test code only, not for use in flight
**
*/

/**
 * Includes
 */
#include "srl_UT.h"
#include "cfe_msg.h"
#include "cfe_core_resourceid_basevalues.h"
#include "cfe_srl_extern_typedefs.h"

/*
 * A method to add an SRL "Subtest"
 *
 * As SRL has a lot of test cases it is helpful to group them.
 * This allows each test routine to reported consistently as "GroupName.TestName"
 *
 * This also implicitly includes a call to SB_ResetUnitTest() as a setup function,
 * so the test routines do _not_ need to do this explicitly on every test case.
 */
#define SRL_UT_ADD_SUBTEST(func) UtTest_AddSubTest(func, SRL_ResetUnitTest, NULL, __func__, #func);

/* Normal dispatching registers the MsgID+CC in order to follow a
 * certain path through a series of switch statements */
#define SRL_UT_MID_DISPATCH(intf) \
    .Method = UT_TaskPipeDispatchMethod_MSG_ID_CC, .MsgId = CFE_SB_MSGID_WRAP_VALUE(CFE_SRL_##intf##_MID)

#define SRL_UT_MSG_DISPATCH(intf, cmd)      SRL_UT_MID_DISPATCH(intf), UT_TPD_SETSIZE(CFE_SRL_##cmd)
#define SRL_UT_CC_DISPATCH(intf, cc, cmd)   SRL_UT_MSG_DISPATCH(intf, cmd), UT_TPD_SETCC(cc)
#define SRL_UT_ERROR_DISPATCH(intf, cc, err) SRL_UT_MID_DISPATCH(intf), UT_TPD_SETCC(cc), UT_TPD_SETERR(err)

/* NOTE: Automatic formatting of this table tends to make it harder to read. */
/* clang-format off */
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_NOOP_CC =
    { SRL_UT_CC_DISPATCH(CMD, CFE_SRL_NOOP_CC, NoopCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_RESET_CONTERS_CC =
    { SRL_UT_CC_DISPATCH(CMD, CFE_SRL_RESET_COUNTERS_CC, ResetCounterCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_GET_HANDLE_STATUS_CC =
    { SRL_UT_CC_DISPATCH(CMD, CFE_SRL_GET_HANDLE_STATUS_CC, GetHandleStatusCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_INIT_HANDLE_CC = 
    { SRL_UT_CC_DISPATCH(CMD, CFE_SRL_INIT_HANDLE_CC, InitHandleCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_CLOSE_HANDLE_CC =
    { SRL_UT_CC_DISPATCH(CMD, CFE_SRL_CLOSE_HANDLE_CC, CloseHandleCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_CONFIG_HANDLE_CC = 
    { SRL_UT_CC_DISPATCH(CMD, CFE_SRL_CONFIG_HANDLE_CC, ConfigHandleCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_SEND_HK =
    { SRL_UT_MSG_DISPATCH(SEND_HK, SendHkCmd) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_INVALID_MID = 
    { .Method = UT_TaskPipeDispatchMethod_MSG_ID_CC, UT_TPD_SETERR(CFE_STATUS_UNKNOWN_MSG_ID) };
static const UT_TaskPipeDispatchId_t UT_TPID_CFE_SRL_CMD_INVALID_CC = 
    { SRL_UT_ERROR_DISPATCH(CMD, -1, CFE_STATUS_BAD_COMMAND_CODE) };
/* clang-format on */

/**
 * Stub function for ApiWrite test
 */
int32 TempTxFunc(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Param) {return CFE_SUCCESS;}
int32 TempRxFunc(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Param) {return CFE_SUCCESS;}

void SRL_ResetUnitTest(void) {
    UT_InitData();
    // CFE_SRL_EarlyInit(); /* Need This? */
}

void UtTest_Setup(void) {
    UT_Init("srl");
    UtPrintf("cFE SRL Unit Test Output File\n\n");

    /* cfe_srl_task.c functions */
    Test_SRL_TaskInit();
    Test_SRL_TaskMain();

    /* cfe_srl_dispatch.c functions */
    Test_SRL_TaskPipe();

    /* cfe_srl_internal.c functions */
    Test_SRL_EarlyInit();

    /* cfe_srl_api.c functions */
    Test_SRL_Api();
}

/*------------------------SRL Task Init Test-----------------------------*/
void Test_SRL_TaskInit_Nominal(void) {
    UtAssert_INT32_EQ(CFE_SRL_TaskInit(), CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 0);
}

void Test_SRL_TaskInit_EVSRegFail(void) {
    int32 ForcedRetVal = -23;

    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, ForcedRetVal);
    UtAssert_INT32_EQ(CFE_SRL_TaskInit(), ForcedRetVal);

    CFE_UtAssert_SYSLOG("%s: Call to CFE_EVS_Register Failed:RC=0x%08X\n");
}

void Test_SRL_TaskInit_CrPipeFail(void) {
    int32 ForcedRetval = -27;
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, ForcedRetval);
    UtAssert_INT32_EQ(CFE_SRL_TaskInit(), ForcedRetval);

    CFE_UtAssert_SYSLOG("%s: Call to CFE_SB_CreatePipe Failed. RC=0x%08X\n");
}

void Test_SRL_TaskInit_SubCmdFail(void) {
    int32 ForcedRetVal = -21;
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, ForcedRetVal);
    UtAssert_INT32_EQ(CFE_SRL_TaskInit(), ForcedRetVal);

    CFE_UtAssert_SYSLOG("%s: Subscribe to Cmds Failed. RC=0x%08X\n");
}

void Test_SRL_TaskInit_SubHkFail(void) {
    int32 ForcedRetVal = -22;
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, ForcedRetVal);
    UtAssert_INT32_EQ(CFE_SRL_TaskInit(), ForcedRetVal);

    CFE_UtAssert_SYSLOG("%s: Subscribe to Cmds Failed. RC=0x%08X\n");
}

void Test_SRL_TaskInit_EVSSendEvtFail(void) {
    int32 ForcedRetval = -24;

    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_SendEvent), 1, ForcedRetval);
    UtAssert_INT32_EQ(CFE_SRL_TaskInit(), ForcedRetval);

    CFE_UtAssert_SYSLOG("%s: Error sending init event. RC=0x%08X\n");
}

void Test_SRL_TaskInit(void) {
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskInit_Nominal);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskInit_EVSRegFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskInit_CrPipeFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskInit_SubCmdFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskInit_SubHkFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskInit_EVSSendEvtFail);
}
/*----------------------End of SRL Task Init Test--------------------------*/

/*-------------------------SRL Task Main Test------------------------------*/
void Test_SRL_TaskMain_Nominal(void) {
    uint32 ExitCode;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    ExitCode = 0;
    UT_SetDataBuffer(UT_KEY(CFE_ES_ExitApp), &ExitCode, sizeof(ExitCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(MsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UtAssert_VOIDCALL(CFE_SRL_TaskMain());

    /* Verify results */
    UtAssert_INT32_EQ(ExitCode, CFE_ES_RunStatus_CORE_APP_RUNTIME_ERROR);
    UtAssert_STUB_COUNT(CFE_ES_ExitApp, 1);
}

void Test_SRL_TaskMain_InitFail(void) {
    uint32_t ExitCode = 0;
    int32 ForcedRetVal = -20;
    UT_SetDataBuffer(UT_KEY(CFE_ES_ExitApp), &ExitCode, sizeof(ExitCode), false);
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, ForcedRetVal);

    UtAssert_VOIDCALL(CFE_SRL_TaskMain());

    /* Verify results */
    UtAssert_INT32_EQ(ExitCode, CFE_ES_RunStatus_CORE_APP_INIT_ERROR);
    /* Since stub doesn't actually cause an exit, will get called twice */
    UtAssert_STUB_COUNT(CFE_ES_ExitApp, 2);
}

void Test_SRL_TaskMain_RcvErr(void) {
    int32 ForcedRetVal = -21;
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, ForcedRetVal);

    UtAssert_VOIDCALL(CFE_SRL_TaskMain());

    /* Verify results */
    CFE_UtAssert_SYSLOG("%s: Error reading cmd pipe,RC=0x%08X\n");
    UtAssert_STUB_COUNT(CFE_ES_ExitApp, 2);
}

void Test_SRL_TaskMain(void) {
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskMain_Nominal);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskMain_InitFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskMain_RcvErr);
}
/*----------------------End of SRL Task Main Test--------------------------*/


/*-------------------------SRL Task Pipe Test------------------------------*/
union {
    CFE_SRL_SendHkCmd_t SendHk;
    CFE_SRL_NoopCmd_t NoopCmd;
    CFE_SRL_ResetCounterCmd_t ResetCmd;
    CFE_SRL_GetHandleStatusCmd_t GetHandleStat;
    CFE_SRL_InitHandleCmd_t InitHandle;
    CFE_SRL_CloseHandleCmd_t CloseHandle;
    CFE_SRL_ConfigHandleCmd_t ConfigHandle;
    CFE_MSG_Message_t Msg;
} UT_CmdBuf;

void Test_SRL_TaskPipe_SendHk(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.SendHk), UT_TPID_CFE_SRL_SEND_HK);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}


void Test_SRL_TaskPipe_Noop(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.NoopCmd), UT_TPID_CFE_SRL_CMD_NOOP_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_NOOP_INF_EID);
}
void Test_SRL_TaskPipe_ResetCounter(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.NoopCmd), UT_TPID_CFE_SRL_CMD_RESET_CONTERS_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_RESET_INF_EID);
}
void Test_SRL_TaskPipe_GetHandleStatus(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.GetHandleStat), UT_TPID_CFE_SRL_CMD_GET_HANDLE_STATUS_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_GET_HANDLE_STATUS_INF_EID);
}

void Test_SRL_TaskPipe_InitHandle(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.GetHandleStat), UT_TPID_CFE_SRL_CMD_INIT_HANDLE_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_INIT_HANDLE_INF_EID);
}
void Test_SRL_TaskPipe_CloseHandle(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.GetHandleStat), UT_TPID_CFE_SRL_CMD_CLOSE_HANDLE_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_CLOSE_HANDLE_INF_EID);
}
void Test_SRL_TaskPipe_ConfigHandle(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.GetHandleStat), UT_TPID_CFE_SRL_CMD_CONFIG_HANDLE_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_CONFIG_HANDLE_INF_EID);
}

void Test_SRL_TaskPipe_BadMsgId(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.NoopCmd), UT_TPID_CFE_SRL_INVALID_MID);
    CFE_UtAssert_EVENTSENT(CFE_SRL_MID_ERR_EID);
    UtAssert_ZERO(CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter);
    UtAssert_BOOL_TRUE(CFE_SRL_Global.HKTlmMsg.Payload.CommandErrorCounter == 1);
}
void Test_SRL_TaskPipe_BadCC(void) {
    UT_CallTaskPipe(CFE_SRL_TaskPipe, CFE_MSG_PTR(UT_CmdBuf), sizeof(UT_CmdBuf.NoopCmd), UT_TPID_CFE_SRL_CMD_INVALID_CC);
    CFE_UtAssert_EVENTSENT(CFE_SRL_CC_ERR_EID);
    UtAssert_ZERO(CFE_SRL_Global.HKTlmMsg.Payload.CommandCounter);
    UtAssert_BOOL_TRUE(CFE_SRL_Global.HKTlmMsg.Payload.CommandErrorCounter == 1);
}

void Test_SRL_TaskPipe(void) {
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_SendHk);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_Noop);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_ResetCounter);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_GetHandleStatus);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_InitHandle);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_CloseHandle);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_ConfigHandle);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_BadMsgId);
    SRL_UT_ADD_SUBTEST(Test_SRL_TaskPipe_BadCC);
}
/*----------------------End of SRL Task Pipe Test--------------------------*/


/*-------------------------SRL internal Test------------------------------*/
void Test_SRL_EarlyInit_Nominal(void) {
    memset(&CFE_SRL_Global, 0xFF, sizeof(CFE_SRL_Global));
    // CFE_SRL_EarlyInit();
    UtAssert_INT32_EQ(CFE_SRL_EarlyInit(), CFE_SUCCESS);

    /* Invoked by PriorInit() */
    UtAssert_STUB_COUNT(OS_MutSemCreate, 1);

    /* If IO Handle Init success, SysLog is invoked */
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, (1 + CFE_SRL_GNRL_DEVICE_NUM));
}

void Test_SRL_EarlyInit_CrMutSemFail(void) {
    UT_SetDeferredRetcode(UT_KEY(OS_MutSemCreate), 1, -1);

    UtAssert_INT32_EQ(CFE_SRL_EarlyInit(), CFE_SRL_PRIOR_INIT_ERR);
    CFE_UtAssert_SYSLOG("%s: Global Handle Mutex create failed.\n");
}

void Test_SRL_EarlyInit_GpioInitFail(void) {
    /* Test gpio init error if gpio is used */
    if (CFE_SRL_TOT_GPIO_NUM) {
        UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, -1);

        UtAssert_BOOL_FALSE(CFE_SRL_EarlyInit() == CFE_SUCCESS);
    }
}

void Test_SRL_EarlyInit_HandleInitFail(void) {
    if (CFE_SRL_GNRL_DEVICE_NUM) {
        UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, -1);

        UtAssert_BOOL_FALSE(CFE_SRL_EarlyInit() == CFE_SUCCESS);
    }
}

void Test_SRL_EarlyInit(void) {
    SRL_UT_ADD_SUBTEST(Test_SRL_EarlyInit_Nominal);
    SRL_UT_ADD_SUBTEST(Test_SRL_EarlyInit_CrMutSemFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_EarlyInit_GpioInitFail);
    SRL_UT_ADD_SUBTEST(Test_SRL_EarlyInit_HandleInitFail);
}
/*----------------------End of SRL internal Test--------------------------*/


/*---------------------------SRL API Test--------------------------------*/
void Test_SRL_ApiWrite(void) {
    CFE_SRL_IO_Handle_t Handle = {0};
    CFE_SRL_IO_Param_t Param = {0};
    Handle.Func.TxFunc = TempTxFunc;
    Handle.Func.RxFunc = NULL;

    uint8_t TempTxBuf = 0;
    Param.TxData = &TempTxBuf;

    /* Nominal case */
    UtAssert_INT32_EQ(CFE_SRL_ApiWrite(&Handle, &Param), CFE_SUCCESS);

    /* Test Bad arguments */
    /* Handle == NULL case */
    UtAssert_INT32_EQ(CFE_SRL_ApiWrite(NULL, &Param), CFE_SRL_BAD_ARGUMENT);

    /* Params->TxData == NULL case */
    Param.TxData = NULL;
    UtAssert_INT32_EQ(CFE_SRL_ApiWrite(&Handle, &Param), CFE_SRL_BAD_ARGUMENT);

    /* Handle->Func.TxFunc case */
    Handle.Func.TxFunc = NULL;
    UtAssert_INT32_EQ(CFE_SRL_ApiWrite(&Handle, &Param), CFE_SRL_BAD_ARGUMENT);
}

void Test_SRL_ApiRead(void) {
    CFE_SRL_IO_Handle_t Handle = {0};
    CFE_SRL_IO_Param_t Param = {0};
    Handle.Func.TxFunc = NULL;
    Handle.Func.RxFunc = TempRxFunc;

    uint8_t TempRxBuf = 0;
    Param.RxData = &TempRxBuf;

    /* Nominal Case */
    UtAssert_INT32_EQ(CFE_SRL_ApiRead(&Handle, &Param), CFE_SUCCESS);

    /* Test Bad arguments */
    /* Handle == NULL case */
    UtAssert_INT32_EQ(CFE_SRL_ApiRead(NULL, &Param), CFE_SRL_BAD_ARGUMENT);

    /* Params->RxData == NULL case */
    Param.RxData = NULL;
    UtAssert_INT32_EQ(CFE_SRL_ApiRead(&Handle, &Param), CFE_SRL_BAD_ARGUMENT);

    /* Handle->Func.RxFunc case */
    Handle.Func.RxFunc = NULL;
    UtAssert_INT32_EQ(CFE_SRL_ApiRead(&Handle, &Param), CFE_SRL_BAD_ARGUMENT);
}

void Test_SRL_ApiGpioSet(void) {
    CFE_SRL_GPIO_Handle_t Handle = {0};
    bool SetVal = 0;

    /* Nominal Case */
    UtAssert_INT32_EQ(CFE_SRL_ApiGpioSet(&Handle, SetVal), CFE_SUCCESS);

    /* Bad Argument */
    UtAssert_INT32_EQ(CFE_SRL_ApiGpioSet(NULL, SetVal), CFE_SRL_BAD_ARGUMENT);

    /* PSP Fail */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, -1);
    UtAssert_BOOL_FALSE(CFE_SRL_ApiGpioSet(&Handle, SetVal) == CFE_SUCCESS);
}

void Test_SRL_ApiGpioGet(void) {
    CFE_SRL_GPIO_Handle_t Handle = {0};
    bool GetVal = 0;

    /* Nominal Case */
    UtAssert_INT32_EQ(CFE_SRL_ApiGpioGet(&Handle, &GetVal), CFE_SUCCESS);

    /* Bad Argument */
    UtAssert_INT32_EQ(CFE_SRL_ApiGpioGet(NULL, &GetVal), CFE_SRL_BAD_ARGUMENT);

    /* PSP Fail */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, -1);
    UtAssert_BOOL_FALSE(CFE_SRL_ApiGpioGet(&Handle, &GetVal) == CFE_SUCCESS);
}

void Test_SRL_Api(void) {
    SRL_UT_ADD_SUBTEST(Test_SRL_ApiWrite);
    SRL_UT_ADD_SUBTEST(Test_SRL_ApiRead);
    SRL_UT_ADD_SUBTEST(Test_SRL_ApiGpioSet);
    SRL_UT_ADD_SUBTEST(Test_SRL_ApiGpioGet);
}
/*------------------------End of SRL API Test----------------------------*/