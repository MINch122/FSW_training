/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM Ground Command-handling functions
 */

/**
 * Preprocessor
 * This is the indicator of w/w.o SOBC architecture
 * If SOBC not used, undef `USE_SOBC`
 */
#define USE_SOBC
#undef USE_SOBC

/*
** Include Files:
*/
#include "payuzuc_task.h"
#include "payuzuc_cmds.h"
#include "payuzuc_msgids.h"
#include "payuzuc_eventids.h"
#include "payuzuc_msg.h"

#include "payuzuc_internal_cfg.h"
#include "payuzuc_interface_cfg.h"
#include "payuzuc_utils.h"
#include "cfe.h"

void PAYUZUC_ConfigurePacket(const void *Payload, void *Packet, uint8 ParamNum, uint8_t Command) {
    if (Packet == NULL) return;
    if (ParamNum != 0 && Payload == NULL) return;

    PAYUZUC_Cmd_t *Cmd = (PAYUZUC_Cmd_t *)Packet;

    Cmd->Packet.StartByte = PAYUZUC_PKT_START_BYTE;
    Cmd->Packet.Command = Command;
    if (ParamNum != 0) {
        memcpy(Cmd->Packet.Params, Payload, ParamNum);
    }
    Cmd->Packet.EndByte = PAYUZUC_PKT_TERMINATE_BYTE;
    
    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUZUC_SendHkCmd(const PAYUZUC_SendHkCmd_t *Msg) {
    PAYUZUC_Data.HkTlm.Payload.CommandCounter = PAYUZUC_Data.CmdCounter;
    PAYUZUC_Data.HkTlm.Payload.CommandErrorCounter = PAYUZUC_Data.ErrCounter;

    for (uint8_t i = 0; i < PAYUZUC_MEMORY_SLOT; i++) {
        PAYUZUC_Data.HkTlm.Payload.MemoryState[i] = PAYUZUC_Data.MemSlotStatus.Entry[i].MemoryState;
        PAYUZUC_Data.HkTlm.Payload.LastImgIdx[i] = PAYUZUC_Data.MemSlotStatus.Entry[i].LastImgIdx;
    }
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUZUC_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUZUC_Data.HkTlm.TelemetryHeader), true);

    // CFE_EVS_SendEvent(PAYUZUC_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUC Send HK Cmd Received.");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC NOOP commands                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_NoopCmd(const PAYUZUC_NoopCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    CFE_EVS_SendEvent(PAYUZUC_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUC Noop Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUZUC_ResetCountersCmd(const PAYUZUC_ResetCountersCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter = 0;
    PAYUZUC_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(PAYUZUC_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUC Reset Counters Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Ping commands                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_PingCmd(const PAYUZUC_PingCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_PING_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_PING_PARAM_SIZE, PAYUZUC_PING_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_PING_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    
    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Set Mode commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_SetModeCmd(const PAYUZUC_SetModeCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_SET_MODE_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_SET_MODE_PARAM_SIZE, PAYUZUC_SET_MODE_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_SET_MODE_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Memory Status commands                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_MemoryStatusCmd(const PAYUZUC_MemoryStatusCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;
    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_MEMORY_STATUS_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(Msg, &Cmd, PAYUZUC_MEMORY_STATUS_PARAM_SIZE, PAYUZUC_MEMORY_STATUS_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_MEMORY_STATUS_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Set Exposure commands                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_SetExposureCmd(const PAYUZUC_SetExposureCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_SET_EXPOSURE_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_SET_EXPOSURE_PARAM_SIZE, PAYUZUC_SET_EXPOSURE_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_SET_EXPOSURE_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Capture commands                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_CaptureCmd(const PAYUZUC_CaptureCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_CAPTURE_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_CAPTURE_PARAM_SIZE, PAYUZUC_CAPTURE_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_CAPTURE_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    /**
     * Clear Memory Slot Status
     */
    PAYUZUC_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = PAYUZUC_DOWNLOAD_NOT_STARTED;
    for (uint8_t i = 0; i < 60; i++) {
        PAYUZUC_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i] = 0x00;
    }
       
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Download commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_DownloadCmd(const PAYUZUC_DownloadCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    PAYUZUC_Cmd_t Cmd = {0,};

    uint8 RxBuf[PAYUZUC_DOWNLOAD_TLM_SIZE] = {0,};

    if (Msg->Payload.PRE != PAYUZUC_DOWNLOAD_THUMBNAIL_FLAG && Msg->Payload.PRE != PAYUZUC_DOWNLOAD_ORIGINAL_FLAG) {
        PAYUZUC_Data.ErrCounter ++;
        return CFE_SUCCESS;
    }    
    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_DOWNLOAD_PARAM_SIZE,
                                PAYUZUC_DOWNLOAD_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = Msg->Payload.PRE ? PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE : PAYUZUC_DOWNLOAD_TLM_SIZE;
    Params.Timeout = 100;
    Params.Interval = 1000*70; // Empirical value

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    uint16_t Line = (Msg->Payload.LN1 << 8) | Msg->Payload.LN2;
    
    /**
     * Open, Write and Close Image file
     */
    int FD = PAYUZUC_OpenFile(Msg->Payload.MEM, Line, 0);
    if (FD < 0) {
        OS_printf("Open error.\n");
    }

    Status = PAYUZUC_WriteToFile(FD, RxBuf, Msg->Payload.PRE ? PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE : PAYUZUC_DOWNLOAD_TLM_SIZE, false);
    if (Status == CFE_SUCCESS) PAYUZUC_SetLineTrue(Msg->Payload.MEM, Line);
    else OS_printf("Write Error. RC = %d\n", Status);

    Status = PAYUZUC_CloseFile(FD);
    if (Status < 0) OS_printf("Close Error.\n");

    /**
     * Update Memory State
     */
    PAYUZUC_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = PAYUZUC_DOWNLOAD_ON_GOING;

    /**
     * Inspection
     */
    PAYUZUC_Inspection(Msg->Payload.MEM);

    /* Rx Data Debugging */
    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\t", RxBuf[i]);
        if (i%10 == 9) OS_printf("\n");
    }
    
    /* State Debugging */
    OS_printf("Line Status\n");
    for (uint8_t i=0; i<60; i++) {
        OS_printf("0x%02X\t",PAYUZUC_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i]);
        if (i%10 == 9) OS_printf("\n");
    }

    /**
     * Store Table State to File
     */
    Status = PAYUZUC_WriteToFile(PAYUZUC_Data.TblHandle, &PAYUZUC_Data.MemSlotStatus, sizeof(PAYUZUC_Memory_Status_t), true);
    if (Status != CFE_SUCCESS) {
        OS_printf("Write Fail.\n");
    }

    return CFE_SUCCESS;
}


CFE_Status_t PAYUZUC_DownloadAllCmd(const PAYUZUC_DownloadAllCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    PAYUZUC_Cmd_t Cmd = {0,};
    uint16_t ErrCnt;

    uint8 RxBuf[PAYUZUC_DOWNLOAD_TLM_SIZE] = {0,};

    const uint16_t TotLine = Msg->Payload.PRE ? PAYUZUC_THUMBNAIL_IMG_LINE_NUM : PAYUZUC_IMG_LINE_NUM;

    if (Msg->Payload.PRE != PAYUZUC_DOWNLOAD_THUMBNAIL_FLAG && Msg->Payload.PRE != PAYUZUC_DOWNLOAD_ORIGINAL_FLAG) {
        PAYUZUC_Data.ErrCounter ++;
        return CFE_SUCCESS;
    }

    if (Msg->Payload.StartLine < 0 || (Msg->Payload.StartLine + Msg->Payload.LineNum) > TotLine) {
        PAYUZUC_Data.ErrCounter ++;
        return CFE_SUCCESS;
    }

    /**
     * Open New file - If already exist, truncate it
     */
    int FD = PAYUZUC_OpenFile(Msg->Payload.MEM, Msg->Payload.StartLine, Msg->Payload.LineNum);

    uint8_t Payload[PAYUZUC_DOWNLOAD_PARAM_SIZE] = {0,};
    Payload[0] = Msg->Payload.MEM;
    Payload[1] = Msg->Payload.PRE;
 
    for (uint16_t line = Msg->Payload.StartLine; line < (Msg->Payload.StartLine + Msg->Payload.LineNum); 
        line ++) {
        Payload[2] = (line >> 8) & 0xFF; // Line MSB
        Payload[3] = line & 0xFF;       // Line LSB
        PAYUZUC_ConfigurePacket(Payload, &Cmd, PAYUZUC_DOWNLOAD_PARAM_SIZE,
                            PAYUZUC_DOWNLOAD_CMD_CODE);
        
        CFE_SRL_IO_Param_t Params = {0,};
        Params.TxData = &Cmd;
        Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
        Params.RxData = &RxBuf;
        Params.RxSize = Msg->Payload.PRE ? PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE : PAYUZUC_DOWNLOAD_TLM_SIZE;
        Params.Timeout = 2000;
        Params.Interval = 1000*70; // Empirical value

        Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
        if (Status != CFE_SUCCESS) {
            ErrCnt ++;
            continue;
        }
        
        /**
         * Write Image Data to file
         */
        Status  = PAYUZUC_WriteToFile(FD, RxBuf, Msg->Payload.PRE ? PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE : PAYUZUC_DOWNLOAD_TLM_SIZE, false);
        if (Status != CFE_SUCCESS) {
            OS_printf("Write Error. RC = %d\n", Status);
            ErrCnt ++;
            continue;
        }
        memset(RxBuf, 0, sizeof(RxBuf));

        /**
         * Update line state
         */
        PAYUZUC_SetLineTrue(Msg->Payload.MEM, line);

        /* Debugging */
        OS_printf("Line %u Download done.\n", line);

    }
    /**
     * When Download Done, Close file
     */
    Status = PAYUZUC_CloseFile(FD);

    /**
     * Update download state
     */
    PAYUZUC_Data.MemSlotStatus.Entry[Msg->Payload.MEM].MemoryState = PAYUZUC_DOWNLOAD_ON_GOING;

    /**
     * Inspection
     */
    PAYUZUC_Inspection(Msg->Payload.MEM);
    
    /* Debugging */
    OS_printf("Line Status\n");
    for (uint8_t i=0; i<60; i++) {
        OS_printf("0x%02X\t",PAYUZUC_Data.MemSlotStatus.Entry[Msg->Payload.MEM].LineState[i]);
        if (i%10 == 9) OS_printf("\n");
    }

    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    /**
     * Store Table State to File
     */
    Status = PAYUZUC_WriteToFile(PAYUZUC_Data.TblHandle, &PAYUZUC_Data.MemSlotStatus, sizeof(PAYUZUC_Memory_Status_t), true);
    if (Status != CFE_SUCCESS) {
        OS_printf("Write Fail!.\n");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Read Register commands                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_ReadRegisterCmd(const PAYUZUC_ReadRegisterCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;

    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_READ_REGISTER_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_READ_REGISTER_PARAM_SIZE, PAYUZUC_READ_REGISTER_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_READ_REGISTER_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Write Register commands                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_WriteRegisterCmd(const PAYUZUC_WriteRegisterCmd_t *Msg) {
    PAYUZUC_Data.CmdCounter++;
    
    int32 Status;
    
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_WRITE_REGISTER_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_WRITE_REGISTER_PARAM_SIZE, PAYUZUC_WRITE_REGISTER_CMD_CODE);

    CFE_SRL_IO_Param_t Params = {0,};
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_WRITE_REGISTER_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    for (int i = 0; i < sizeof(RxBuf); i++) {
        OS_printf("0x%02X\n", RxBuf[i]);
    }
    return CFE_SUCCESS;
}