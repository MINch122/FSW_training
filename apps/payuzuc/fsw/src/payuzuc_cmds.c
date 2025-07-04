/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM Ground Command-handling functions
 */

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
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_PING_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_PING_PARAM_SIZE, PAYUZUC_PING_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_PING_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Set Mode commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_SetModeCmd(const PAYUZUC_SetModeCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_SET_MODE_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_SET_MODE_PARAM_SIZE, PAYUZUC_SET_MODE_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_SET_MODE_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Memory Status commands                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_MemoryStatusCmd(const PAYUZUC_MemoryStatusCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_MEMORY_STATUS_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(Msg, &Cmd, PAYUZUC_MEMORY_STATUS_PARAM_SIZE, PAYUZUC_MEMORY_STATUS_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_MEMORY_STATUS_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Set Exposure commands                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_SetExposureCmd(const PAYUZUC_SetExposureCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_SET_EXPOSURE_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_SET_EXPOSURE_PARAM_SIZE, PAYUZUC_SET_EXPOSURE_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_SET_EXPOSURE_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Capture commands                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_CaptureCmd(const PAYUZUC_CaptureCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_CAPTURE_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_CAPTURE_PARAM_SIZE, PAYUZUC_CAPTURE_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_CAPTURE_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Download commands                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_DownloadCmd(const PAYUZUC_DownloadCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};

    PAYUZUC_Cmd_t Cmd = {0,};

    uint8 RxBufThumb[PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE] = {0,};
    uint8 RxBuf[PAYUZUC_DOWNLOAD_TLM_SIZE] = {0,};

    if (Msg->Payload.PRE == PAYUZUC_DOWNLOAD_THUMBNAIL_FLAG) {    

        PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_DOWNLOAD_PARAM_SIZE,
                                PAYUZUC_DOWNLOAD_CMD_CODE);

        Params.TxData = &Cmd;
        Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
        Params.RxData = &RxBufThumb;
        Params.RxSize = PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE;
        Params.Timeout = 10;

        Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    }

    else if (Msg->Payload.PRE == PAYUZUC_DOWNLOAD_ORIGINAL_FLAG){

        PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_DOWNLOAD_PARAM_SIZE,
                                PAYUZUC_DOWNLOAD_CMD_CODE);

        Params.TxData = &Cmd;
        Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
        Params.RxData = &RxBuf;
        Params.RxSize = PAYUZUC_DOWNLOAD_TLM_SIZE;
        Params.Timeout = 10;

        Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    }
    
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }
    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Read Register commands                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_ReadRegisterCmd(const PAYUZUC_ReadRegisterCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_READ_REGISTER_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_READ_REGISTER_PARAM_SIZE, PAYUZUC_READ_REGISTER_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_READ_REGISTER_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }
    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC Write Register commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUC_WriteRegisterCmd(const PAYUZUC_WriteRegisterCmd_t *Msg) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8 RxBuf[PAYUZUC_WRITE_REGISTER_TLM_SIZE] = {0,};

    PAYUZUC_ConfigurePacket(&Msg->Payload, &Cmd, PAYUZUC_WRITE_REGISTER_PARAM_SIZE, PAYUZUC_WRITE_REGISTER_CMD_CODE);
    
    Params.TxData = &Cmd;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = &RxBuf;
    Params.RxSize = PAYUZUC_WRITE_REGISTER_TLM_SIZE;
    Params.Timeout = 10;

    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}