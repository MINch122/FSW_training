/**
 * \file
 *   This file contains the source code for the SLT FTP Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "ftp_cmds.h"
#include "ftp_msgids.h"
#include "ftp_eventids.h"
#include "ftp_msg.h"
#include "ftp_task.h"
#include "ftp_utils.h"

#include "ftp_internal_cfg.h"
#include "ftp_interface_cfg.h"

#include "cfe.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t FTP_SendHkCmd(const FTP_SendHkCmd_t *Msg) {
    // housekeeping 
    // ....
    return CFE_SUCCESS;
}

CFE_Status_t FTP_SendBcnCmd(const FTP_SendBcnCmd_t *Msg) {
    // beacon
    // ....
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FTP NOOP commands                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t FTP_NoopCmd(const FTP_NoopCmd_t *Msg) {

    FTP_Data.CmdCounter ++;

    CFE_EVS_SendInfo(FTP_NOOP_INF_EID, "FTP Noop Command Receiveed.");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FTP Reset Counter commands                                                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t FTP_ResetCountersCmd(const FTP_ResetCountersCmd_t *Msg) {

    FTP_Data.CmdCounter = 0;
    FTP_Data.ErrCounter = 0;

    CFE_EVS_SendInfo(FTP_RESET_INF_EID, "FTP Noop Reset counter Command Receiveed.");

    return CFE_SUCCESS;
}


CFE_Status_t FTP_SendFileCmd(const FTP_SendFileCmd_t *Msg) {
    int32 OsStatus;
    const FTP_SendFileCmd_Payload_t Payload = Msg->Payload;

    if (Payload.StartByte >= Payload.EndByte) {
        return CFE_STATUS_VALIDATION_FAILURE;
    }

    uint32_t TotBytes = Payload.EndByte - Payload.StartByte + 1; // Total bytes to read
    uint32_t SendBytes = 0;
    uint8_t RdBuf[FTP_MAX_CHUNK_SIZE] = {0,};

    OsStatus = OS_OpenCreate(&FTP_Data.FileId, Payload.FileName, OS_FILE_FLAG_NONE, OS_READ_ONLY);
    if (OsStatus != OS_SUCCESS) {
        FTP_HandleReport(OsStatus, RPT_RETTYPE_OSAL, FTP_SEND_FILE_CC, NULL, 0);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
    OS_printf("Open Success.\n");

    OsStatus = OS_lseek(FTP_Data.FileId, Payload.StartByte, OS_SEEK_SET);
    if (OsStatus < OS_SUCCESS) {
        OS_printf("Os Status: %d\n", OsStatus);
        FTP_HandleReport(OsStatus, RPT_RETTYPE_OSAL, FTP_SEND_FILE_CC, NULL, 0);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
    OS_printf("lseek Success.\n");

    /* Local Timer Start */
    OsStatus = OS_TimerSet(FTP_Data.TimerId, (1000 * 1000 * 60 * Payload.TimeLimit), 0);
    if (OsStatus != OS_SUCCESS) {
        FTP_HandleReport(OsStatus, RPT_RETTYPE_OSAL, FTP_SEND_FILE_CC, NULL, 0);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    FTP_Data.RunFlag = true;
    int32 BytesRead = 0;
    uint32_t SeqCnt = 0;

    /* File Transmission loop */
    while (FTP_Data.RunFlag) {
        if (SendBytes >= TotBytes) {
            FTP_Data.RunFlag = false;
            break;
        }

        BytesRead = OS_read(FTP_Data.FileId, RdBuf, 
                            (TotBytes - SendBytes >= FTP_MAX_CHUNK_SIZE) ? 
                            FTP_MAX_CHUNK_SIZE : (TotBytes - SendBytes));

        if (BytesRead == 0) {
            /* Finished reading file */
            FTP_Data.RunFlag = false;
            OS_close(FTP_Data.FileId);
        }
        else if (BytesRead < 0) {
            /* Error reading file */
            FTP_Data.RunFlag = false;
            OS_close(FTP_Data.FileId);

            /* Report */
            FTP_HandleReport(BytesRead, RPT_RETTYPE_OSAL, FTP_SEND_FILE_CC, NULL, 0);
            return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }
        else {
            OS_printf("Sequence Count: %u || BytesRead: %d\n", SeqCnt, BytesRead);
            /* Send data to GS */
            if (SeqCnt == 0) {
                CFE_MSG_SetSegmentationFlag(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), CFE_MSG_SegFlag_First);
            }
            else {
                CFE_MSG_SetSegmentationFlag(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), CFE_MSG_SegFlag_Continue);
            }

            CFE_MSG_SetSequenceCount(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), SeqCnt);
            CFE_MSG_SetSize(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), sizeof(FTP_Data.Chunk.TelemetryHeader) + BytesRead);
            memcpy(FTP_Data.Chunk.Payload.Bytes, RdBuf, BytesRead);

            /* Test using TO Lab */
            // CFE_SB_TimeStampMsg(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader));
            // CFE_SB_TransmitMsg(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), false);
            /* End of To test */

            /* Actual Transmission to GS */
            CFE_SRL_ApiTransactionCSP(CSP_NODE_GS_KISS, 14, &FTP_Data.Chunk, sizeof(FTP_Data.Chunk.TelemetryHeader) + BytesRead, NULL, 0);
            
            memset(RdBuf, 0, sizeof(RdBuf));
            SeqCnt ++;
        }
        /* Time interval for each chunk */
        OS_TaskDelay(FTP_CHUNK_SLEEP_MS);

        SendBytes += BytesRead;
    }

    /**
     * Last packet
     * There is no file data, just CCSDS tlm hdr
     */
    CFE_MSG_SetSegmentationFlag(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), CFE_MSG_SegFlag_Last);
    CFE_MSG_SetSequenceCount(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), SeqCnt);
    CFE_MSG_SetSize(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), sizeof(FTP_Data.Chunk.TelemetryHeader));

    /* Test using TO Lab */
    // CFE_SB_TimeStampMsg(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader));
    // CFE_SB_TransmitMsg(CFE_MSG_PTR(FTP_Data.Chunk.TelemetryHeader), false);
    /* End of To test */

    /* Actual Transmission to GS */
    CFE_SRL_ApiTransactionCSP(CSP_NODE_GS_KISS, 14, &FTP_Data.Chunk.TelemetryHeader, sizeof(FTP_Data.Chunk.TelemetryHeader), NULL, 0);
    
    OS_TaskDelay(1000);

    /**
     * Report about statistics (?)
     * Final Sequence Count & Total Send bytes
     */
    uint32_t Report[2] = {SeqCnt, SendBytes};
    FTP_HandleReport(CFE_SUCCESS, RPT_RETTYPE_SUCCESS, FTP_SEND_FILE_CC, Report, sizeof(Report));

    CFE_EVS_SendInfo(FTP_SEND_FILE_INF_EID, "FTP Send file command Received. File: %s || Send Bytes: %u", Payload.FileName, SendBytes);

    return CFE_SUCCESS;
}