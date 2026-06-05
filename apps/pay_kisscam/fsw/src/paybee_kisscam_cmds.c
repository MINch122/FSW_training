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
#include <unistd.h>


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
    paybee_kisscam_Data.CmdCounter++;
    static const char NoopReport[] = "Yosi In Space";

    paybee_kisscam_APP_printf("paybee_kisscam: NOOP report requested\n");

    paybee_kisscam_HandleSuccess(paybee_kisscam_NOOP_CC, (void *)NoopReport, sizeof(NoopReport));

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

    paybee_kisscam_HandleSuccess(paybee_kisscam_RESET_COUNTERS_CC, Cnts, sizeof(Cnts));

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
    uint8 RxBuf[paybee_kisscam_PING_TLM_SIZE] = {0,};

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

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_PING_CC);

    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");
    
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
    uint8 RxBuf[paybee_kisscam_SET_MODE_TLM_SIZE] = {0,};

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

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_SET_MODE_CC);

    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     OS_printf("0x%02X\t", RxBuf[i]);
    // }
    // OS_printf("\n");

    // CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "Set Mode");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");

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
    uint8 RxBuf[paybee_kisscam_MEMORY_STATUS_TLM_SIZE] = {0,};

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

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_MEMORY_STATUS_CC);

    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     OS_printf("0x%02X\t", RxBuf[i]);
    // }
    // OS_printf("\n");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");

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
    uint8 RxBuf[paybee_kisscam_SET_EXPOSURE_TLM_SIZE] = {0,};

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

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_SET_EXPOSURE_CC);

    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     OS_printf("0x%02X\t", RxBuf[i]);
    // }
    // OS_printf("\n");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");

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
    uint8 RxBuf[paybee_kisscam_CAPTURE_TLM_SIZE] = {0,};

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

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_CAPTURE_CC);

    /**
     * Clear Memory Slot Status
     */
    paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = paybee_kisscam_DOWNLOAD_NOT_STARTED;
    for (uint8_t i = 0; i < 60; i++) {
        paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i] = 0x00;
    }
       
    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     OS_printf("0x%02X\t", RxBuf[i]);
    // }
    // OS_printf("\n");

    // CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "Capture");

    
    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");
    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Download commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_DownloadCmd(const paybee_kisscam_DownloadCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    int32 Status;
    paybee_kisscam_Cmd_t Cmd = {0,};

    uint8 RxBuf[paybee_kisscam_DOWNLOAD_TLM_SIZE] = {0,};

    if (Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_THUMBNAIL_FLAG && Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_ORIGINAL_FLAG) {
        paybee_kisscam_Data.ErrCounter ++;
        return CFE_SUCCESS;
    }    
    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_DOWNLOAD_PARAM_SIZE,
                                paybee_kisscam_DOWNLOAD_CMD_CODE);

    // CFE_SRL_IO_Param_t Params = {0,};
    // Params.TxData = &Cmd;
    // Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    // Params.RxData = &RxBuf;
    // Params.RxSize = Msg->Payload.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE : paybee_kisscam_DOWNLOAD_TLM_SIZE;
    // Params.Timeout = 100;
    // Params.Interval = 1000*70; // Empirical value

    // Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    // if (Status != CFE_SUCCESS) {
    //     paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_DOWNLOAD_CC, Params.RxData, Params.ReadBytes);
    // }
    // else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
    //     paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_DOWNLOAD_CC);
    //     paybee_kisscam_Data.DeviceErrCounter ++;
    //     return CFE_SUCCESS; 
    // }

    // else paybee_kisscam_HandleSuccess(paybee_kisscam_DOWNLOAD_CC, Params.RxData, Params.ReadBytes);

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_DOWNLOAD_CC);

    uint16_t Line = (Msg->Payload.LN1 << 8) | Msg->Payload.LN2;
    
    /**
     * Open, Write and Close Image file
     */
    int FD = paybee_kisscam_OpenFile(Msg->Payload.MEM, Line, 0);
    if (FD < 0) {
        OS_printf("Open error.\n");
    }

    Status = paybee_kisscam_WriteToFile(FD, RxBuf, Msg->Payload.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE : paybee_kisscam_DOWNLOAD_TLM_SIZE, false);
    if (Status == CFE_SUCCESS) paybee_kisscam_SetLineTrue(Msg->Payload.MEM, Line);
    else OS_printf("Write Error. RC = %d\n", Status);

    Status = paybee_kisscam_CloseFile(FD);
    if (Status < 0) OS_printf("Close Error.\n");

    /**
     * Update Memory State
     */
    paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = paybee_kisscam_DOWNLOAD_ON_GOING;

    /**
     * Inspection
     */
    paybee_kisscam_Inspection(Msg->Payload.MEM);

    /* Rx Data Debugging */
    // for (int i = 0; i < sizeof(RxBuf); i++) {
    //     OS_printf("0x%02X\t", RxBuf[i]);
    //     if (i%10 == 9) OS_printf("\n");
    // }
    
    /* State Debugging */
    // OS_printf("Line Status\n");
    // for (uint8_t i=0; i<60; i++) {
    //     OS_printf("0x%02X\t",paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i]);
    //     if (i%10 == 9) OS_printf("\n");
    // }

    /**
     * Store Table State to File
     */
    Status = paybee_kisscam_WriteToFile(paybee_kisscam_Data.TblHandle, &paybee_kisscam_Data.MemSlotStatus, sizeof(paybee_kisscam_Memory_Status_t), true);
    if (Status != CFE_SUCCESS) {
        OS_printf("Write Fail.\n");
    }

    
    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");

    return CFE_SUCCESS;
}


/*****************************************************
 * This Command has special Report structure
 * Does not report the serial Rx data
 * Just report the ErrCnt (Error counter) 
 * Which is occured during whole download procedure
 *****************************************************/
CFE_Status_t paybee_kisscam_DownloadAllCmd(const paybee_kisscam_DownloadAllCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;

    CFE_EVS_SendEvent(paybee_kisscam_DOWNLOAD_START_INF_EID, CFE_EVS_EventType_INFORMATION, "paybee_kisscam_Donwload Start.");

    int32 Status;
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint16_t ErrCnt = 0;

    uint8 RxBuf[paybee_kisscam_DOWNLOAD_TLM_SIZE] = {0,};

    const uint16_t TotLine = Msg->Payload.PRE ? paybee_kisscam_THUMBNAIL_IMG_LINE_NUM : paybee_kisscam_IMG_LINE_NUM;

    if (Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_THUMBNAIL_FLAG && Msg->Payload.PRE != paybee_kisscam_DOWNLOAD_ORIGINAL_FLAG) {
        paybee_kisscam_Data.ErrCounter ++;
        return CFE_SUCCESS;
    }

    if (Msg->Payload.StartLine < 0 || (Msg->Payload.StartLine + Msg->Payload.LineNum) > TotLine) {
        paybee_kisscam_Data.ErrCounter ++;
        return CFE_SUCCESS;
    }

    /**
     * Open New file - If already exist, truncate it
     */
    int FD = paybee_kisscam_OpenFile(Msg->Payload.MEM, Msg->Payload.StartLine, Msg->Payload.LineNum);

    uint8_t Payload[paybee_kisscam_DOWNLOAD_PARAM_SIZE] = {0,};
    Payload[0] = Msg->Payload.MEM;
    Payload[1] = Msg->Payload.PRE;
 
    for (uint16_t line = Msg->Payload.StartLine; line < (Msg->Payload.StartLine + Msg->Payload.LineNum); line ++) 
        {
        Payload[2] = (line >> 8) & 0xFF; // Line MSB
        Payload[3] = line & 0xFF;       // Line LSB
        paybee_kisscam_ConfigurePacket(Payload, &Cmd, paybee_kisscam_DOWNLOAD_PARAM_SIZE,
                            paybee_kisscam_DOWNLOAD_CMD_CODE);
        
        CFE_SRL_IO_Param_t Params = {0,};
        Params.TxData = &Cmd;
        Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
        Params.RxData = &RxBuf;
        Params.RxSize = Msg->Payload.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE : paybee_kisscam_DOWNLOAD_TLM_SIZE;
        Params.Timeout = 200;
        Params.Interval = 100000; // Empirical value 70ms, Margin for stability

        Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
        if (Status != CFE_SUCCESS) {
            // ErrCnt ++;
            paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_DOWNLOAD_ALL_CC, Params.RxData, Params.ReadBytes);
            return CFE_SUCCESS;
        }
        else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
            paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_DOWNLOAD_ALL_CC);
            // ErrCnt ++;
            paybee_kisscam_Data.DeviceErrCounter ++;
            return CFE_SUCCESS;
        }
        
        /**
         * Write Image Data to file
         */
        Status  = paybee_kisscam_WriteToFile(FD, RxBuf, Msg->Payload.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE : paybee_kisscam_DOWNLOAD_TLM_SIZE, false);
        if (Status != CFE_SUCCESS) {
            // OS_printf("Write Error. RC = %d\n", Status);
            ErrCnt ++;
            break;
        }
        memset(RxBuf, 0, sizeof(RxBuf));

        /**
         * Update line state
         */
        paybee_kisscam_SetLineTrue(Msg->Payload.MEM, line);

        /* Debugging */
        // OS_printf("Line %u Download done.\n", line);

    }
    /**
     * When Download Done, Close file
     */
    Status = paybee_kisscam_CloseFile(FD);

    /**
     * Update download state
     */
    paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = paybee_kisscam_DOWNLOAD_ON_GOING;

    /**
     * Inspection
     */
    paybee_kisscam_Inspection(Msg->Payload.MEM);
    
    /* Debugging */
    // OS_printf("Line Status\n");
    // for (uint8_t i=0; i<60; i++) {
    //     OS_printf("0x%02X\t",paybee_kisscam_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i]);
    //     if (i%10 == 9) OS_printf("\n");
    // }

    if (Status != CFE_SUCCESS) {
        ErrCnt ++;
        paybee_kisscam_Data.ErrCounter ++;
        goto report;
    }

    /**
     * Store Table State to File
     */
    Status = paybee_kisscam_WriteToFile(paybee_kisscam_Data.TblHandle, &paybee_kisscam_Data.MemSlotStatus, sizeof(paybee_kisscam_Memory_Status_t), true);
    if (Status != CFE_SUCCESS) {
        ErrCnt ++;
        // OS_printf("Write Fail!.\n");
        // goto report;
    }

report: {
    paybee_kisscam_ReportTlm_t Report = {0, };
    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(paybee_kisscam_REPORT_TLM_MID),
                    sizeof(paybee_kisscam_ReportTlm_t));
    Report.Report.MsgID = paybee_kisscam_CMD_MID;
    Report.Report.CommandCode = paybee_kisscam_DOWNLOAD_ALL_CC;
    Report.Report.ReturnType = (ErrCnt == 0) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
    Report.Report.ReturnCode = Status;
    Report.Report.ReturnDataSize = sizeof(ErrCnt);
    memcpy(Report.Report.ReturnValue, &ErrCnt, sizeof(ErrCnt));
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);
}


    /* for test - CMD Packet을 OBC에서 출력  */
    OS_printf("[KissCAM] Tx Cmd: ");
    for (int i = 0; i < paybee_kisscam_CMD_PKT_SIZE; i++) {
        OS_printf("0x%02X ", Cmd.Bytes[i]);
    }
    OS_printf("\n");

    /* for test - TLM Packet을 OBC에서 출력 */
    OS_printf("[KissCAM] Rx Tlm: ");
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X ", RxBuf[i]);
    }
    OS_printf("\n");
    
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
//         OS_printf("0x%02X\t", RxBuf[i]);
//     }
//     OS_printf("\n");

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
    uint8 RxBuf[paybee_kisscam_READ_REGISTER_TLM_SIZE] = {0,};

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

    paybee_kisscam_Transaction(&Cmd, RxBuf, paybee_kisscam_WRITE_REGISTER_CC);

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\t", RxBuf[i]);
    }
    OS_printf("\n");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam Write Register commands                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t paybee_kisscam_WriteRegisterCmd(const paybee_kisscam_WriteRegisterCmd_t *Msg) {
    paybee_kisscam_Data.CmdCounter++;
    
    int32 Status;
    
    paybee_kisscam_Cmd_t Cmd = {0,};
    uint8 RxBuf[paybee_kisscam_WRITE_REGISTER_TLM_SIZE] = {0,};

    paybee_kisscam_ConfigurePacket(&Msg->Payload, &Cmd, paybee_kisscam_WRITE_REGISTER_PARAM_SIZE, paybee_kisscam_WRITE_REGISTER_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = paybee_kisscam_WRITE_REGISTER_TLM_SIZE;
    Params.Timeout = 700;

    Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_WRITE_REGISTER_CC, Params.RxData, Params.ReadBytes);
    }
    else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
        paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_WRITE_REGISTER_CC);
        paybee_kisscam_Data.DeviceErrCounter ++;
    }

    else paybee_kisscam_HandleSuccess(paybee_kisscam_WRITE_REGISTER_CC, Params.RxData, Params.ReadBytes);

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\t", RxBuf[i]);
    }
    OS_printf("\n");

    return CFE_SUCCESS;
}