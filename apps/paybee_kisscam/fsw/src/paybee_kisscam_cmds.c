/**
 * \file
 *   This file contains the source code for the paybee_kisscam Ground Command-handling functions
 */

/**
 * Preprocessor
 * This is the indicator of w/w.o SOBC architecture
 * If SOBC not used, undef `USE_SOBC`
 */
#define USE_DEMO
#undef USE_DEMO

/*
** Include Files:
*/
#include "paybee_kisscam_task.h"
#include "paybee_kisscam_cmds.h"
#include "paybee_kisscam_msgids.h"
#include "paybee_kisscam_eventids.h"
#include "paybee_kisscam_msg.h"

#include "paybee_kisscam_internal_cfg.h"
#include "paybee_kisscam_interface_cfg.h"
#include "paybee_kisscam_utils.h"
#include "cfe.h"
#include <fcntl.h>
#include <libaec.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

#define PAYBEE_KISSCAM_MIN_RX_BUF_SIZE 9u
#define PAYBEE_KISSCAM_RX_BUF_SIZE(ExpectedSize) \
    (((ExpectedSize) > PAYBEE_KISSCAM_MIN_RX_BUF_SIZE) ? (ExpectedSize) : PAYBEE_KISSCAM_MIN_RX_BUF_SIZE)

static void paybee_kisscam_ReportTransaction(uint8 CC,
                                             const paybee_kisscam_TransactionResult_t *Result,
                                             const void *RxData)
{
    paybee_kisscam_SendReport(CC, Result->ReturnType, Result->ReturnCode,
                              RxData, Result->ReadSize);
}

static void paybee_kisscam_ReportAppError(uint8 CC, int32 ReturnCode)
{
    paybee_kisscam_Data.ErrCounter++;
    paybee_kisscam_SendReport(CC, RPT_RETTYPE_APP, ReturnCode, NULL, 0);
}

static void paybee_kisscam_ReportDownloadPhase(uint8 CC, uint8 Phase,
                                               const paybee_kisscam_TransactionResult_t *Result,
                                               const void *RxData, size_t RxSize)
{
    uint8 ReportData[RPT_RET_VALUE_BUF_SIZE] = {0};
    uint8 ReturnType = RPT_RETTYPE_SUCCESS;
    int32 ReturnCode = CFE_SUCCESS;
    size_t CopySize = 0;

    if (Result != NULL)
    {
        ReturnType = Result->ReturnType;
        ReturnCode = Result->ReturnCode;
    }

    ReportData[0] = Phase;
    if (RxData != NULL && RxSize > 0)
    {
        CopySize = RxSize;
        if (CopySize > sizeof(ReportData) - 1)
        {
            CopySize = sizeof(ReportData) - 1;
        }
        memcpy(&ReportData[1], RxData, CopySize);
    }

    /* Byte 0 is the phase. Any following bytes are the unclassified camera response. */
    paybee_kisscam_SendReport(CC, ReturnType, ReturnCode, ReportData, CopySize + 1);
}

static bool paybee_kisscam_IsRetryableDownloadError(
    const paybee_kisscam_TransactionResult_t *Result)
{
    return Result->ReturnType == RPT_RETTYPE_CFE &&
           (Result->ReturnCode == CFE_SRL_PARTIAL_READ_ERR ||
            Result->ReturnCode == CFE_SRL_TIMEOUT);
}

static paybee_kisscam_TransactionResult_t paybee_kisscam_DownloadTransaction(
    const paybee_kisscam_Cmd_t *Cmd, void *RxData, size_t RxCapacity, uint8 CC, uint16 Line)
{
    uint8 RetryCount = 0;
    paybee_kisscam_TransactionResult_t Result;

    do
    {
        memset(RxData, 0, RxCapacity);
        Result = paybee_kisscam_Transaction(Cmd, RxData, RxCapacity, CC);
        if (Result.ReturnType == RPT_RETTYPE_SUCCESS ||
            !paybee_kisscam_IsRetryableDownloadError(&Result) ||
            RetryCount >= paybee_kisscam_DOWNLOAD_MAX_RETRIES)
        {
            break;
        }

        RetryCount++;
        PAYBEE_KISSCAM_APP_printf("KissCAM download retry: line=%u attempt=%u/%u status=0x%08lX\n",
                                   Line, RetryCount, paybee_kisscam_DOWNLOAD_MAX_RETRIES,
                                   (unsigned long)Result.ReturnCode);
    } while (true);

    return Result;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
// CFE_Status_t paybee_kisscam_SendHkCmd(const paybee_kisscam_SendHkCmd_t *Msg) {

//     paybee_kisscam_Cmd_t Cmd = {0,};
//     uint8 RxBuf[paybee_kisscam_PING_TLM_SIZE] = {0,};
//     uint8_t PingArg = 0;

//     paybee_kisscam_ConfigurePacket(&PingArg, &Cmd, paybee_kisscam_PING_PARAM_SIZE, paybee_kisscam_PING_CMD_CODE);

//     paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_PING_CC);

//     paybee_kisscam_Data.BcnTlm.Payload.Mode = RxBuf[2];

//     for (uint8_t i = 0; i < paybee_kisscam_MEMORY_SLOT; i++) {
//         paybee_kisscam_Data.BcnTlm.Payload.MemoryState[i] = paybee_kisscam_Data.MemSlotStatus.Entry[i].MemoryState;
//         paybee_kisscam_Data.BcnTlm.Payload.LastImgIdx[i] = paybee_kisscam_Data.MemSlotStatus.Entry[i].LastImgIdx;
//     }
//     CFE_SB_TimeStampMsg(CFE_MSG_PTR(paybee_kisscam_Data.BcnTlm.TelemetryHeader));
//     CFE_SB_TransmitMsg(CFE_MSG_PTR(paybee_kisscam_Data.BcnTlm.TelemetryHeader), true);

//     // CFE_EVS_SendEvent(paybee_kisscam_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "paybee_kisscam Send HK Cmd Received.");

//     return CFE_SUCCESS;
// }

// CFE_Status_t paybee_kisscam_SendBcnCmd(const paybee_kisscam_SendBcnCmd_t *Msg) {

//     paybee_kisscam_Cmd_t Cmd = {0,};
//     uint8 RxBuf[paybee_kisscam_PING_TLM_SIZE] = {0,};
//     uint8_t PingArg = 0;

//     paybee_kisscam_ConfigurePacket(&PingArg, &Cmd, paybee_kisscam_PING_PARAM_SIZE, paybee_kisscam_PING_CMD_CODE);

//     paybee_kisscam_TransactionWithoutReport(&Cmd, RxBuf, paybee_kisscam_PING_CC);

//     paybee_kisscam_Data.BcnTlm.Payload.Mode = RxBuf[2];

//     for (uint8_t i = 0; i < paybee_kisscam_MEMORY_SLOT; i++) {
//         paybee_kisscam_Data.BcnTlm.Payload.MemoryState[i] = paybee_kisscam_Data.MemSlotStatus.Entry[i].MemoryState;
//         paybee_kisscam_Data.BcnTlm.Payload.LastImgIdx[i] = paybee_kisscam_Data.MemSlotStatus.Entry[i].LastImgIdx;
//     }
//     CFE_SB_TimeStampMsg(CFE_MSG_PTR(paybee_kisscam_Data.BcnTlm.TelemetryHeader));
//     CFE_SB_TransmitMsg(CFE_MSG_PTR(paybee_kisscam_Data.BcnTlm.TelemetryHeader), true);

//     // CFE_EVS_SendEvent(paybee_kisscam_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "paybee_kisscam Send HK Cmd Received.");

//     return CFE_SUCCESS;
// }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam NOOP commands                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_NoopCmd(const paybee_kisscam_NoopCmd_t *Msg) {
    static const char NoopReport[] = "PAYBEE_KISSCAM NOOP CMD: YOSI IN SPACE";

    paybee_kisscam_Data.CmdCounter++;

    paybee_kisscam_SendReport(paybee_kisscam_NOOP_CC, RPT_RETTYPE_SUCCESS,
                              CFE_SUCCESS, NoopReport, sizeof(NoopReport));

    CFE_EVS_SendEvent(paybee_kisscam_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "paybee_kisscam Noop Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t paybee_kisscam_ResetCountersCmd(const paybee_kisscam_ResetCountersCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter = 0;
    paybee_kisscam_Data.ErrCounter = 0;
    paybee_kisscam_Data.DeviceErrCounter = 0;

    uint8_t Cnts[2] = {paybee_kisscam_Data.CmdCounter, paybee_kisscam_Data.ErrCounter};

    paybee_kisscam_SendReport(paybee_kisscam_RESET_COUNTERS_CC, RPT_RETTYPE_SUCCESS,
                              CFE_SUCCESS, Cnts, sizeof(Cnts));

    CFE_EVS_SendEvent(paybee_kisscam_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "paybee_kisscam Reset Counters Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Ping commands                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_PingCmd(const paybee_kisscam_PingCmd_t *Msg) {
    // Tlm is bigger than error pkt

    paybee_kisscam_Data.CmdCounter++;

    // int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_PING_TLM_SIZE)] = {0,};

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_PING_PARAM_SIZE, paybee_kisscam_PING_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = paybee_kisscam_PING_TLM_SIZE;
    // Params.Timeout = 100;

    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_PING_CC, Params.RxData, Params.ReadBytes);
    // }

    // /**
    //  * Handle Error packet
    //  */
    // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
    //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_PING_CC);
    //     paybee_kisscam_Data.DeviceErrCounter ++;
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_PING_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_PING_CC);
    paybee_kisscam_ReportTransaction(paybee_kisscam_PING_CC, &Result, RxBuf);

    /* for test - CMD Packet을 OBC에서 출력  */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", Cmd.Bytes[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");
    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Set Mode commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_SetModeCmd(const paybee_kisscam_SetModeCmd_t *Msg) {
    
    paybee_kisscam_Data.CmdCounter++;

    // int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_SET_MODE_TLM_SIZE)] = {0,};

    if (Msg->Payload.MD > 2) {
        paybee_kisscam_ReportAppError(paybee_kisscam_SET_MODE_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_SET_MODE_PARAM_SIZE, paybee_kisscam_SET_MODE_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = paybee_kisscam_SET_MODE_TLM_SIZE;
    // Params.Timeout = 700;

    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_SET_MODE_CC, Params.RxData, Params.ReadBytes);
    // }
    // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
    //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_SET_MODE_CC);
    //     paybee_kisscam_Data.DeviceErrCounter ++;
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_SET_MODE_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_SET_MODE_CC);
    paybee_kisscam_ReportTransaction(paybee_kisscam_SET_MODE_CC, &Result, RxBuf);

    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
    // }
    // PAYBEE_KISSCAM_APP_printf("\n");

    // CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "Set Mode");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", Cmd.Bytes[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Memory Status commands                                             */
/* @deprecated  Not used function                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_MemoryStatusCmd(const paybee_kisscam_MemoryStatusCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;
    // int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_MEMORY_STATUS_TLM_SIZE)] = {0,};

    paybee_kisscam_ConfigurePacket(Msg, &Cmd, paybee_kisscam_MEMORY_STATUS_PARAM_SIZE, paybee_kisscam_MEMORY_STATUS_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = paybee_kisscam_MEMORY_STATUS_TLM_SIZE;
    // Params.Timeout = 700;
    
    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_MEMORY_STATUS_CC, Params.RxData, Params.ReadBytes);
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_MEMORY_STATUS_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_MEMORY_STATUS_CC);
    paybee_kisscam_ReportTransaction(paybee_kisscam_MEMORY_STATUS_CC, &Result, RxBuf);

    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
    // }
    // PAYBEE_KISSCAM_APP_printf("\n");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", Cmd.Bytes[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Set Exposure commands                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_SetExposureCmd(const paybee_kisscam_SetExposureCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    // int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_SET_EXPOSURE_TLM_SIZE)] = {0,};

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_SET_EXPOSURE_PARAM_SIZE, paybee_kisscam_SET_EXPOSURE_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = paybee_kisscam_SET_EXPOSURE_TLM_SIZE;
    // Params.Timeout = 700;

    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_SET_EXPOSURE_CC, Params.RxData, Params.ReadBytes);
    // }
    // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
    //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_SET_EXPOSURE_CC);
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_SET_EXPOSURE_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_SET_EXPOSURE_CC);
    paybee_kisscam_ReportTransaction(paybee_kisscam_SET_EXPOSURE_CC, &Result, RxBuf);

    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
    // }
    // PAYBEE_KISSCAM_APP_printf("\n");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", Cmd.Bytes[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Capture commands                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_CaptureCmd(const paybee_kisscam_CaptureCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    // int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_CAPTURE_TLM_SIZE)] = {0,};

    if (Msg->Payload.MEM >= paybee_kisscam_MEMORY_SLOT) {
        paybee_kisscam_ReportAppError(paybee_kisscam_CAPTURE_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_CAPTURE_PARAM_SIZE, paybee_kisscam_CAPTURE_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = paybee_kisscam_CAPTURE_TLM_SIZE;
    // Params.Timeout = 700;

    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_CAPTURE_CC, Params.RxData, Params.ReadBytes);
    // }
    // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
    //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_CAPTURE_CC);
    //     paybee_kisscam_Data.DeviceErrCounter ++;
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_CAPTURE_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_CAPTURE_CC);

    /**
     * Clear Memory Slot Status
     */
    if (Result.ReturnType == RPT_RETTYPE_SUCCESS &&
        Result.ReadSize == paybee_kisscam_CAPTURE_TLM_SIZE) {
        paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = paybee_kisscam_DOWNLOAD_NOT_STARTED;
        for (uint8_t i = 0; i < 60; i++) {
            paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i] = 0x00;
        }
    }
    paybee_kisscam_ReportTransaction(paybee_kisscam_CAPTURE_CC, &Result, RxBuf);
       
    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
    // }
    // PAYBEE_KISSCAM_APP_printf("\n");

    // CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "Capture");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", Cmd.Bytes[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X ", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");
    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Download commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_DownloadCmd(const paybee_kisscam_DownloadCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[paybee_kisscam_DOWNLOAD_TLM_SIZE] = {0,};
    size_t ExpectedRxSize;
    uint16_t TotLine;
    uint16_t Line;
    int FD = -1;
    int SavedErrno;
    int32 Status;
    paybee_kisscam_TransactionResult_t Result = {CFE_SUCCESS, RPT_RETTYPE_SUCCESS, 0};

    if (Msg->Payload.MEM >= paybee_kisscam_MEMORY_SLOT ||
        (Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_THUMBNAIL_FLAG &&
         Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_ORIGINAL_FLAG)) {
        paybee_kisscam_ReportAppError(paybee_kisscam_DOWNLOAD_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    TotLine = Msg->Payload.PRE ? paybee_kisscam_THUMBNAIL_IMG_LINE_NUM : paybee_kisscam_IMG_LINE_NUM;
    Line = ((uint16_t)Msg->Payload.LN1 << 8) | Msg->Payload.LN2;
    if (Line >= TotLine) {
        paybee_kisscam_ReportAppError(paybee_kisscam_DOWNLOAD_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    ExpectedRxSize = Msg->Payload.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE
                                      : paybee_kisscam_DOWNLOAD_TLM_SIZE;
    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_DOWNLOAD_PARAM_SIZE,
                                    paybee_kisscam_DOWNLOAD_CMD_CODE);
    paybee_kisscam_ReportDownloadPhase(paybee_kisscam_DOWNLOAD_CC,
                                       paybee_kisscam_RPT_PHASE_STARTED, NULL, NULL, 0);
    Result = paybee_kisscam_DownloadTransaction(&Cmd, RxBuf, sizeof(RxBuf),
                                                paybee_kisscam_DOWNLOAD_CC, Line);
    if (Result.ReturnType != RPT_RETTYPE_SUCCESS) goto report;
    if (Result.ReadSize != ExpectedRxSize) {
        PAYBEE_KISSCAM_APP_printf("KissCAM download response size mismatch: line=%u expected=%lu read=%lu\n",
                                   Line, (unsigned long)ExpectedRxSize,
                                   (unsigned long)Result.ReadSize);
        goto report;
    }

    FD = paybee_kisscam_OpenFile(Msg->Payload.MEM, Line, 0);
    if (FD < 0) {
        SavedErrno = errno;
        Result.ReturnType = RPT_RETTYPE_OSAL;
        Result.ReturnCode = (SavedErrno != 0) ? -SavedErrno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        Result.ReadSize = 0;
        paybee_kisscam_Data.ErrCounter++;
        goto report;
    }

    Status = paybee_kisscam_WriteToFile(FD, RxBuf, ExpectedRxSize, false);
    if (Status != CFE_SUCCESS) {
        Result.ReturnType = RPT_RETTYPE_OSAL;
        Result.ReturnCode = Status;
        Result.ReadSize = 0;
        paybee_kisscam_Data.ErrCounter++;
    }
    Status = paybee_kisscam_CloseFile(FD);
    FD = -1;
    if (Status != CFE_SUCCESS && Result.ReturnType == RPT_RETTYPE_SUCCESS) {
        Result.ReturnType = RPT_RETTYPE_OSAL;
        Result.ReturnCode = Status;
        Result.ReadSize = 0;
        paybee_kisscam_Data.ErrCounter++;
    }

    if (Result.ReturnType == RPT_RETTYPE_SUCCESS) {
        paybee_kisscam_SetLineTrue(Msg->Payload.MEM, Line);
        paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = paybee_kisscam_DOWNLOAD_ON_GOING;
        paybee_kisscam_Inspection(Msg->Payload.MEM, TotLine);
        Status = paybee_kisscam_WriteToFile(paybee_kisscam_Data.TblHandle,
                                             &paybee_kisscam_Data.MemSlotStatus,
                                             sizeof(paybee_kisscam_Memory_Status_t), true);
        if (Status != CFE_SUCCESS) {
            Result.ReturnType = RPT_RETTYPE_OSAL;
            Result.ReturnCode = Status;
            Result.ReadSize = 0;
            paybee_kisscam_Data.ErrCounter++;
        }
    }

report:
    if (FD >= 0) paybee_kisscam_CloseFile(FD);
    paybee_kisscam_ReportDownloadPhase(paybee_kisscam_DOWNLOAD_CC,
                                       paybee_kisscam_RPT_PHASE_FINISHED, &Result,
                                       (Result.ReadSize == ExpectedRxSize) ? NULL : RxBuf,
                                       (Result.ReadSize == ExpectedRxSize) ? 0 : Result.ReadSize);
    return CFE_SUCCESS;
}


/*****************************************************
 * Download-all emits one STARTED and one FINISHED report.
 * Complete image responses are stored locally. An incomplete or non-image
 * response is appended to the FINISHED report for ground-side interpretation.
 *****************************************************/
CFE_Status_t paybee_kisscam_DownloadAllCmd(const paybee_kisscam_DownloadAllCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    PAYBEE_KISSCAM_APP_printf("paybee_kisscam_Download Start.\n");

    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[paybee_kisscam_DOWNLOAD_TLM_SIZE] = {0,};
    uint8_t Payload[paybee_kisscam_DOWNLOAD_PARAM_SIZE] = {0,};
    uint16_t TotLine;
    uint16_t LineCount;
    uint16_t EndLine;
    size_t ExpectedRxSize;
    int FD = -1;
    int SavedErrno;
    int32 Status;
    paybee_kisscam_TransactionResult_t Result = {CFE_SUCCESS, RPT_RETTYPE_SUCCESS, 0};

    if (Msg->Payload.MEM >= paybee_kisscam_MEMORY_SLOT ||
        (Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_THUMBNAIL_FLAG &&
         Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_ORIGINAL_FLAG)) {
        paybee_kisscam_ReportAppError(paybee_kisscam_DOWNLOAD_ALL_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    TotLine = Msg->Payload.PRE ? paybee_kisscam_THUMBNAIL_IMG_LINE_NUM : paybee_kisscam_IMG_LINE_NUM;
    LineCount = (Msg->Payload.LineNum == 0) ? 1 : Msg->Payload.LineNum;
    if (Msg->Payload.StartLine >= TotLine || LineCount > (TotLine - Msg->Payload.StartLine)) {
        paybee_kisscam_ReportAppError(paybee_kisscam_DOWNLOAD_ALL_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }
    EndLine = Msg->Payload.StartLine + LineCount;
    ExpectedRxSize = Msg->Payload.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE
                                      : paybee_kisscam_DOWNLOAD_TLM_SIZE;
    paybee_kisscam_ReportDownloadPhase(paybee_kisscam_DOWNLOAD_ALL_CC,
                                       paybee_kisscam_RPT_PHASE_STARTED, NULL, NULL, 0);

    /**
     * Open New file - If already exist, truncate it
     */
    FD = paybee_kisscam_OpenFile(Msg->Payload.MEM, Msg->Payload.StartLine, LineCount);
    if (FD < 0) {
        SavedErrno = errno;
        Result.ReturnType = RPT_RETTYPE_OSAL;
        Result.ReturnCode = (SavedErrno != 0) ? -SavedErrno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        Result.ReadSize = 0;
        paybee_kisscam_Data.ErrCounter++;
        goto report;
    }

    Payload[0] = Msg->Payload.MEM;
    Payload[1] = Msg->Payload.PRE;
 
    for (uint16_t line = Msg->Payload.StartLine; line < EndLine; line++) {
        Payload[2] = (line >> 8) & 0xFF; // Line MSB
        Payload[3] = line & 0xFF;       // Line LSB
        paybee_kisscam_ConfigurePacket(Payload, &Cmd, paybee_kisscam_DOWNLOAD_PARAM_SIZE,
                            paybee_kisscam_DOWNLOAD_CMD_CODE);

        Result = paybee_kisscam_DownloadTransaction(&Cmd, RxBuf, sizeof(RxBuf),
                                                    paybee_kisscam_DOWNLOAD_ALL_CC, line);
        if (Result.ReturnType != RPT_RETTYPE_SUCCESS) break;
        if (Result.ReadSize != ExpectedRxSize) {
            PAYBEE_KISSCAM_APP_printf("KissCAM download response size mismatch: line=%u expected=%lu read=%lu\n",
                                       line, (unsigned long)ExpectedRxSize,
                                       (unsigned long)Result.ReadSize);
            break;
        }
        
        /**
         * Write Image Data to file
         */
        Status = paybee_kisscam_WriteToFile(FD, RxBuf, ExpectedRxSize, false);
        if (Status != CFE_SUCCESS) {
            Result.ReturnType = RPT_RETTYPE_OSAL;
            Result.ReturnCode = Status;
            Result.ReadSize = 0;
            paybee_kisscam_Data.ErrCounter++;
            break;
        }

        /**
         * Update line state
         */
        paybee_kisscam_SetLineTrue(Msg->Payload.MEM, line);

        /* Debugging */
        PAYBEE_KISSCAM_APP_printf("Line %u Download done.\n", line);

    }
    /**
     * When Download Done, Close file
     */
    Status = paybee_kisscam_CloseFile(FD);
    FD = -1;

    if (Status != CFE_SUCCESS && Result.ReturnType == RPT_RETTYPE_SUCCESS) {
        Result.ReturnType = RPT_RETTYPE_OSAL;
        Result.ReturnCode = Status;
        Result.ReadSize = 0;
        paybee_kisscam_Data.ErrCounter++;
    }

    /**
     * Update download state
     */
    paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = paybee_kisscam_DOWNLOAD_ON_GOING;

    /* A failed range remains ON_GOING so completed-line state can be resumed. */
    if (Result.ReturnType == RPT_RETTYPE_SUCCESS) {
        paybee_kisscam_Inspection(Msg->Payload.MEM, TotLine);
    }
    
    /* Debugging */
    // PAYBEE_KISSCAM_APP_printf("Line Status\n");
    // for (uint8_t i=0; i<60; i++) {
    //     PAYBEE_KISSCAM_APP_printf("0x%02X\t",paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i]);
    //     if (i%10 == 9) PAYBEE_KISSCAM_APP_printf("\n");
    // }

    /**
     * Store Table State to File
     */
    Status = paybee_kisscam_WriteToFile(paybee_kisscam_Data.TblHandle, &paybee_kisscam_Data.MemSlotStatus, sizeof(paybee_kisscam_Memory_Status_t), true);
    if (Status != CFE_SUCCESS && Result.ReturnType == RPT_RETTYPE_SUCCESS) {
        Result.ReturnType = RPT_RETTYPE_OSAL;
        Result.ReturnCode = Status;
        Result.ReadSize = 0;
        paybee_kisscam_Data.ErrCounter++;
    }

report:
    if (FD >= 0) paybee_kisscam_CloseFile(FD);
    paybee_kisscam_ReportDownloadPhase(paybee_kisscam_DOWNLOAD_ALL_CC,
                                       paybee_kisscam_RPT_PHASE_FINISHED, &Result,
                                       (Result.ReadSize == ExpectedRxSize) ? NULL : RxBuf,
                                       (Result.ReadSize == ExpectedRxSize) ? 0 : Result.ReadSize);
    return CFE_SUCCESS;
}

// CFE_Status_t paybee_kisscam_DownloadAll2Cmd(const paybee_kisscam_DownloadAllCmd_t *Msg) {
//     paybee_kisscam_Data.CmdCounter++;

//     OS_MutSemTake(paybee_kisscam_Data.MutId);
//     paybee_kisscam_Data.DownTaskArg = Msg->Payload;
//     OS_MutSemGive(paybee_kisscam_Data.MutId);

//     CFE_ES_CreateChildTask(&paybee_kisscam_Data.DownTaskId, paybee_kisscam_CHILD_TASK_NAME, paybee_kisscam_DownloadTask,
//                             CFE_ES_TASK_STACK_ALLOCATE, paybee_kisscam_CHILD_STACK_SIZE(2), paybee_kisscam_CHILD_PRIORITY, 0);

//     return CFE_SUCCESS;
// }


// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// /*                                                                            */
// /* paybee_kisscam Mosaic commands                                                    */
// /*                                                                            */
// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// CFE_Status_t paybee_kisscam_MosaicCmd(const paybee_kisscam_MosaicCmd_t *Msg) {
//     paybee_kisscam_Data.CmdCounter++;

//     // int32 Status;

//     paybee_kisscam_Cmd_t Cmd = {0,};
//     uint8 RxBuf[paybee_kisscam_MOSAIC_TLM_SIZE] = {0,};

//     paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_MOSAIC_PARAM_SIZE, paybee_kisscam_MOSAIC_CMD_CODE);

//     // CFE_SRL_IO_Param_t Params = {0,};
//     // Params.TxData = &Cmd;
//     // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
//     // Params.RxData = RxBuf;
//     // Params.RxSize = paybee_kisscam_MOSAIC_TLM_SIZE;
//     // Params.Timeout = 700;

//     // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
//     // if (Status != CFE_SUCCESS) {
//     //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_MOSAIC_CC, Params.RxData, Params.ReadBytes);
//     // }
//     // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
//     //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_MOSAIC_CC);
//     //     paybee_kisscam_Data.DeviceErrCounter ++;
//     // }

//     // else paybee_kisscam_HandleSuccess(paybee_kisscam_MOSAIC_CC, Params.RxData, Params.ReadBytes);

//     paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_READ_REGISTER_CC);

//     for (uint8_t i = 0; i < sizeof(RxBuf); i++) {
//         PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
//     }
//     PAYBEE_KISSCAM_APP_printf("\n");

//     CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "Mosaic");
//     return CFE_SUCCESS;
// }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Read Register commands                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_ReadRegisterCmd(const paybee_kisscam_ReadRegisterCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    // int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_READ_REGISTER_TLM_SIZE)] = {0,};

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_READ_REGISTER_PARAM_SIZE, paybee_kisscam_READ_REGISTER_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = paybee_kisscam_READ_REGISTER_TLM_SIZE;
    // Params.Timeout = 700;

    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_READ_REGISTER_CC, Params.RxData, Params.ReadBytes);
    // }
    // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
    //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_READ_REGISTER_CC);
    //     paybee_kisscam_Data.DeviceErrCounter ++;
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_READ_REGISTER_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_READ_REGISTER_CC);
    paybee_kisscam_ReportTransaction(paybee_kisscam_READ_REGISTER_CC, &Result, RxBuf);

    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Write Register commands                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_WriteRegisterCmd(const paybee_kisscam_WriteRegisterCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYBEE_KISSCAM_RX_BUF_SIZE(paybee_kisscam_WRITE_REGISTER_TLM_SIZE)] = {0,};

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_WRITE_REGISTER_PARAM_SIZE, paybee_kisscam_WRITE_REGISTER_CMD_CODE);

    paybee_kisscam_TransactionResult_t Result =
        paybee_kisscam_Transaction(&Cmd, RxBuf, sizeof(RxBuf), paybee_kisscam_WRITE_REGISTER_CC);
    paybee_kisscam_ReportTransaction(paybee_kisscam_WRITE_REGISTER_CC, &Result, RxBuf);

    for (int i = 0; i < sizeof(RxBuf); i++) {
        PAYBEE_KISSCAM_APP_printf("0x%02X\t", RxBuf[i]);
    }
    PAYBEE_KISSCAM_APP_printf("\n");

    return CFE_SUCCESS;


    
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Image Compress commands                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static bool paybee_kisscam_ImageCompressWrite(int FD, const void *Data, size_t Size, uint16_t Line, const char *Tag, const char *Path) {
    ssize_t Written = write(FD, Data, Size);
    if (Written != (ssize_t)Size) {
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "[KissCAM] %s write failed line=%u path=%s ret=%zd errno=%d", Tag, Line, Path, Written, errno);
        return false;
    }
    return true;
}

CFE_Status_t paybee_kisscam_ImageCompressCmd(const paybee_kisscam_ImageCompressCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    char InPath[128] = {0};
    char OutPath[128] = {0};
    int InFD = -1;
    int OutFD = -1;
    uint8_t MemSlot = Msg->Payload.MEM;
    uint8_t TargetIdx = Msg->Payload.TargetIdx;
    uint8_t line_buf[648];
    uint8_t comp_buf[1024];
    bool ResultOk = true;
    uint8 ReportType = RPT_RETTYPE_OSAL;
    int32 ReportCode = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    if (MemSlot >= paybee_kisscam_MEMORY_SLOT) {
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "[KissCAM] ImageCompressCmd invalid memory slot %u", MemSlot);
        paybee_kisscam_ReportAppError(paybee_kisscam_IMAGE_COMPRESS_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    uint8_t last_idx = paybee_kisscam_Data.MemSlotStatus.Entry[MemSlot].LastImgIdx;
    if (TargetIdx >= last_idx) {
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "[KissCAM] ImageCompressCmd target idx %u is out of range. Last valid idx in slot %u is %d", 
                          TargetIdx, MemSlot, last_idx > 0 ? last_idx - 1 : -1);
        paybee_kisscam_ReportAppError(paybee_kisscam_IMAGE_COMPRESS_CC, CFE_STATUS_RANGE_ERROR);
        return CFE_SUCCESS;
    }

    snprintf(InPath, sizeof(InPath), "%s%u_%u_%03u-%03u", paybee_kisscam_IMG_PATH,
             MemSlot, TargetIdx, 0, 479);
    snprintf(OutPath, sizeof(OutPath), "./cf/sdcard/compressed_image_%u_%u.bin", MemSlot, TargetIdx);
    // snprintf(OutPath, sizeof(OutPath), "/root/0609_KISSCAM_COMP/obc/cf/sdcard/compressed_image_%u_%u.bin", MemSlot, target_idx);

    
    InFD = open(InPath, O_RDONLY);
    if (InFD < 0) {
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "[KissCAM] Compress Error: Cannot open input file %s errno=%d", InPath, errno);
        ReportCode = (errno != 0) ? -errno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        paybee_kisscam_Data.ErrCounter++;
        paybee_kisscam_SendReport(paybee_kisscam_IMAGE_COMPRESS_CC, RPT_RETTYPE_OSAL,
                                  ReportCode, NULL, 0);
        return CFE_SUCCESS;
    }

    OutFD = open(OutPath, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (OutFD < 0) {
        int OpenErrno = errno;
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "[KissCAM] Compress Error: Cannot open output file %s errno=%d", OutPath, OpenErrno);
        close(InFD);
        ReportCode = (OpenErrno != 0) ? -OpenErrno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        paybee_kisscam_Data.ErrCounter++;
        paybee_kisscam_SendReport(paybee_kisscam_IMAGE_COMPRESS_CC, RPT_RETTYPE_OSAL,
                                  ReportCode, NULL, 0);
        return CFE_SUCCESS;
    }

    CFE_EVS_SendEvent(paybee_kisscam_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "[KissCAM] Image Compress Start. Target: %s Out: %s", InPath, OutPath);

    for (uint16_t line = 0; line < 480; line++) {
        int32 bytes_read = read(InFD, line_buf, sizeof(line_buf));
        if (bytes_read != sizeof(line_buf)) {
            CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "[KissCAM] Compress Error: Read error at line %u size=%d", line, bytes_read);
            ResultOk = false;
            ReportCode = (bytes_read < 0 && errno != 0) ? -errno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            break;
        }

        struct aec_stream strm;
        memset(&strm, 0, sizeof(strm));

        strm.bits_per_sample = 8;
        strm.block_size = 16;
        // strm.rsi = 128;
        // strm.flags = 0;
        strm.rsi = 40;
        strm.flags = AEC_DATA_PREPROCESS;

        strm.next_in = &line_buf[7];
        strm.avail_in = 640;
        strm.next_out = comp_buf;
        strm.avail_out = sizeof(comp_buf);

        int AecStatus = aec_encode_init(&strm);
        if (AecStatus != AEC_OK) {
            CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "[KissCAM] Compress Error: aec_encode_init failed at line %u", line);
            ResultOk = false;
            ReportType = RPT_RETTYPE_LIB;
            ReportCode = AecStatus;
            break;
        }

        AecStatus = aec_encode(&strm, AEC_FLUSH);
        if (AecStatus != AEC_OK) {
            CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "[KissCAM] Compress Error: aec_encode failed at line %u RC=%d",
                              line, AecStatus);
            aec_encode_end(&strm);
            ResultOk = false;
            ReportType = RPT_RETTYPE_LIB;
            ReportCode = AecStatus;
            break;
        }
        size_t compressed_size = strm.total_out;
        aec_encode_end(&strm);

        uint16_t line_id = line;
        if (!paybee_kisscam_ImageCompressWrite(OutFD, &line_id, sizeof(line_id), line, "header", OutPath)) {
            ResultOk = false;
            ReportCode = (errno != 0) ? -errno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            break;
        }
        if (!paybee_kisscam_ImageCompressWrite(OutFD, comp_buf, compressed_size, line, "compressed data", OutPath)) {
            ResultOk = false;
            ReportCode = (errno != 0) ? -errno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            break;
        }
    }

    if (ResultOk) {
        if (fsync(OutFD) != 0) {
            CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "[KissCAM] Compress Error: fsync failed on %s errno=%d", OutPath, errno);
            ResultOk = false;
            ReportCode = (errno != 0) ? -errno : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }
    }

    close(InFD);
    close(OutFD);

    if (!ResultOk) {
        unlink(OutPath);
        paybee_kisscam_Data.ErrCounter++;
        paybee_kisscam_SendReport(paybee_kisscam_IMAGE_COMPRESS_CC, ReportType,
                                  ReportCode, NULL, 0);
        return CFE_SUCCESS;
    }

    paybee_kisscam_SendReport(paybee_kisscam_IMAGE_COMPRESS_CC, RPT_RETTYPE_SUCCESS,
                              CFE_SUCCESS, NULL, 0);
    CFE_EVS_SendEvent(paybee_kisscam_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "[KissCAM] Image Compress Done. Output: %s", OutPath);
    return CFE_SUCCESS;
}
