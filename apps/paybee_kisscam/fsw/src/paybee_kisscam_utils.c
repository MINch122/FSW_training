#include "paybee_kisscam_internal_cfg.h"
#include "paybee_kisscam_interface_cfg.h"
#include "paybee_kisscam_tblstruct.h"
#include "paybee_kisscam_eventids.h"
#include "paybee_kisscam_task.h"
#include "paybee_kisscam_utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "rpt_interface_cfg.h"

CFE_Status_t paybee_kisscam_SendReport(uint8_t CC, uint8 ReturnType, int32 ReturnCode,
                                       const void *Data, size_t DataSize)
{
    paybee_kisscam_ReportTlm_t *BufPtr =
        (paybee_kisscam_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(paybee_kisscam_ReportTlm_t));
    uint16_t CopySize = 0;

    if (BufPtr == NULL)
    {
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "KissCAM RPT allocation failed: CC=0x%02X", CC);
        return CFE_SB_BUF_ALOC_ERR;
    }

    memset(BufPtr, 0, sizeof(*BufPtr));

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                     CFE_SB_ValueToMsgId(paybee_kisscam_REPORT_TLM_MID),
                     sizeof(paybee_kisscam_ReportTlm_t)) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "KissCAM RPT init failed: CC=0x%02X", CC);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    BufPtr->Report.MsgID       = paybee_kisscam_CMD_MID;
    BufPtr->Report.CommandCode = CC;
    BufPtr->Report.ReturnType  = ReturnType;
    BufPtr->Report.ReturnCode  = ReturnCode;

    if (Data != NULL && DataSize > 0)
    {
        CopySize = (DataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : (uint16_t)DataSize;
        memcpy(BufPtr->Report.ReturnValue, Data, CopySize);
    }
    BufPtr->Report.ReturnDataSize = CopySize;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    CFE_Status_t Status = CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true);
    if (Status != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "KissCAM RPT transmit failed: CC=0x%02X RC=0x%08lX",
                          CC, (unsigned long)Status);
    }

    return Status;
}

void paybee_kisscam_SetLineTrue(uint8_t MemSlot, uint16_t Line) {
    if(paybee_kisscam_Data.MemSlotStatus.Entry[MemSlot].MemoryState == paybee_kisscam_DOWNLOAD_DONE) return;
    
    paybee_kisscam_Data.MemSlotStatus.Entry[MemSlot].LineState[Line / 8] |= (1 << (Line % 8));

    return;
}


int paybee_kisscam_OpenTblFile(void) {
    // int32 Status;
    int ID;

    ID = open(paybee_kisscam_TBL_PATH, O_CREAT | O_RDWR, 0666);
    // PAYBEE_KISSCAM_APP_printf("ID: %d\n",ID);
    return ID;
}


int paybee_kisscam_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum) {
    int ID;
    char Path[64] = {0, };

    /**
     * One line Download
     */
    if (LineNum == 0) {
        sprintf(Path, "%s%u_%u_%03u", paybee_kisscam_IMG_PATH, MemorySlot, 
            paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine);    
    }
    else sprintf(Path, "%s%u_%u_%03u-%03u", paybee_kisscam_IMG_PATH, MemorySlot, 
                paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine, StartLine + LineNum - 1);
    
    // PAYBEE_KISSCAM_APP_printf("Path: %s\n", Path);
    
    ID = open(Path, O_CREAT| O_TRUNC | O_WRONLY, 0666);
    return ID;
}


int32 paybee_kisscam_WriteToFile(int ID, void *Data, size_t Size, bool IsTbl) {
    int32 Status;
    if (ID < 0 || Data == NULL) {
        return -1;
    }

    if (IsTbl) {
        lseek(ID, 0, SEEK_SET);
        Status = write(ID, Data, Size);
        if (Status != Size) return -1;
        Status = fsync(ID);
        if (Status != 0) return -2;
        return CFE_SUCCESS;
    }
    else {
        Status = write(ID, Data, Size);
        if (Status != Size) return -1;
        Status = fsync(ID);
        if (Status != 0) return -2;
        return CFE_SUCCESS;
    }
}


int32 paybee_kisscam_ReadFile(int ID, void *Data, size_t Size) {
    if (ID < 0 || Data == NULL) {
        return -1;
    }

    return read(ID, Data, Size);
}


int32 paybee_kisscam_CloseFile(int ID) {
    if (ID < 0) {
        return CFE_SUCCESS;
    }

    return close(ID);
}

// 유실 발생하면 인덱스가 안오름
// void paybee_kisscam_Inspection(uint8_t MemorySlot) {
//     for (uint8_t i = 0; i < 60; i++) {
//         if (paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] != 0xFF) {
//             return;
//         }
//     }
//     // PAYBEE_KISSCAM_APP_printf("Memory Slot %u Download Done.\n", MemorySlot);
//     paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].MemoryState = paybee_kisscam_DOWNLOAD_DONE;
//     paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx ++;
//     for (uint8_t i = 0; i < 60; i++) {
//         paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] = 0;
//     }
//     젼
//     return;
// }

// 완화 버젼
void paybee_kisscam_Inspection(uint8_t MemorySlot) {
    bool is_incomplete = false;
    for (uint8_t i = 0; i < 60; i++) {
        if (paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] != 0xFF) {
            is_incomplete = true;
            break;
        }
    }

    if (is_incomplete) {
        CFE_EVS_SendEvent(paybee_kisscam_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "[KissCAM] Download for MemorySlot %u finished with missing lines.", MemorySlot);
    }

    paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].MemoryState = paybee_kisscam_DOWNLOAD_DONE;
    paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx ++;
    for (uint8_t i = 0; i < 60; i++) {
        paybee_kisscam_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] = 0;
    }
    
    return;
}




/***********************************************
 * 
 * Error Handling Function
 * There are several error handling function
 * Refer the description of each function
 * 
 ***********************************************/

/***********************************************
 * 
 * Packet Configuration Util func
 * 
 ***********************************************/
void paybee_kisscam_ConfigurePacket(const void *Payload, void *Packet, uint8 ParamNum, uint8_t Command) {
    if (Packet == NULL) return;
    if (ParamNum != 0 && Payload == NULL) return;

    paybee_kisscam_Cmd_t *Cmd = (paybee_kisscam_Cmd_t *)Packet;

    Cmd->Packet.StartByte = paybee_kisscam_PKT_START_BYTE;
    Cmd->Packet.Command = Command;
    if (ParamNum != 0) {
        memcpy(Cmd->Packet.Params, Payload, ParamNum);
    }
    Cmd->Packet.EndByte = paybee_kisscam_PKT_TERMINATE_BYTE;
    
    return;
}

/***********************************************
 * 
 * Download task util function
 * @deprecated not used
 * 
 ***********************************************/
// void paybee_kisscam_DownloadTask(void) {

//     OS_MutSemTake(paybee_kisscam_Data.MutId);
//     paybee_kisscam_DownloadAll_Payload_t DownLoadInfo = paybee_kisscam_Data.DownTaskArg;
//     OS_MutSemGive(paybee_kisscam_Data.MutId);

//     int32 Status;
//     paybee_kisscam_Cmd_t Cmd = {0,};
//     uint8_t RxBuf[paybee_kisscam_DOWNLOAD_TLM_SIZE] = {0};
//     uint16_t ErrCnt = 0;

//     const uint16_t TotLine = DownLoadInfo.PRE ? paybee_kisscam_THUMBNAIL_IMG_LINE_NUM : paybee_kisscam_IMG_LINE_NUM;;

//     /* Validate the args */
//     if (DownLoadInfo.PRE != paybee_kisscam_DOWNLOAD_THUMBNAIL_FLAG && DownLoadInfo.PRE != paybee_kisscam_DOWNLOAD_ORIGINAL_FLAG) {
//         paybee_kisscam_Data.ErrCounter ++;
//         Status = CFE_STATUS_VALIDATION_FAILURE;
//         goto report;
//     }

//     if (DownLoadInfo.StartLine < 0 || (DownLoadInfo.StartLine + DownLoadInfo.LineNum) > TotLine) {
//         paybee_kisscam_Data.ErrCounter ++;
//         Status = CFE_STATUS_VALIDATION_FAILURE;
//         goto report;
//     }

//     int FD = paybee_kisscam_OpenFile(DownLoadInfo.MEM, DownLoadInfo.StartLine, DownLoadInfo.LineNum);

//     uint8_t Payload[paybee_kisscam_DOWNLOAD_PARAM_SIZE] = {0,};
//     Payload[0] = DownLoadInfo.MEM;
//     Payload[1] = DownLoadInfo.PRE;

//     for (uint16_t line = DownLoadInfo.StartLine; line < (DownLoadInfo.StartLine + DownLoadInfo.LineNum); 
//         line ++) {
//         Payload[2] = (line >> 8) & 0xFF; // Line MSB
//         Payload[3] = line & 0xFF;       // Line LSB
//         paybee_kisscam_ConfigurePacket(Payload, &Cmd, paybee_kisscam_DOWNLOAD_PARAM_SIZE,
//                             paybee_kisscam_DOWNLOAD_CMD_CODE);
        
//         CFE_SRL_IO_Param_t Params = {0,};
//         Params.TxData = &Cmd;
//         Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
//         Params.RxData = &RxBuf;
//         Params.RxSize = DownLoadInfo.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE : paybee_kisscam_DOWNLOAD_TLM_SIZE;
//         Params.Timeout = 200;
//         Params.Interval = 80000; // Empirical value 70ms, Margin for stability

//         Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
//         if (Status != CFE_SUCCESS) {
//             ErrCnt ++;
//             paybee_kisscam_HandleErrorSerial(Status, paybee_kisscam_DOWNLOAD_ALL_CC, Params.RxData, Params.ReadBytes);
//             break;
//         }
//         else if (RxBuf[1] == paybee_kisscam_TLM_ERR_FLAG) {
//             paybee_kisscam_HandleErrorPacket(RxBuf, Params.ReadBytes, paybee_kisscam_DOWNLOAD_ALL_CC);
//             ErrCnt ++;
//             paybee_kisscam_Data.DeviceErrCounter ++;
//             break;
//         }
        
//         /**
//          * Write Image Data to file
//          */
//         Status  = paybee_kisscam_WriteToFile(FD, RxBuf, DownLoadInfo.PRE ? paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE : paybee_kisscam_DOWNLOAD_TLM_SIZE, false);
//         if (Status != CFE_SUCCESS) {
//             // PAYBEE_KISSCAM_APP_printf("Write Error. RC = %d\n", Status);
//             ErrCnt ++;
//             continue;
//         }
//         memset(RxBuf, 0, sizeof(RxBuf));

//         /**
//          * Update line state
//          */
//         paybee_kisscam_SetLineTrue(DownLoadInfo.MEM, line);

//         /* Debugging */
//         // PAYBEE_KISSCAM_APP_printf("%s:Line %u Download done.\n", __func__, line);

//     }
//     /**
//      * When Download Done, Close file
//      */
//     Status = paybee_kisscam_CloseFile(FD);
//     if (Status != CFE_SUCCESS) {
//         ErrCnt ++;
//         paybee_kisscam_Data.ErrCounter ++;
//         goto report;
//     }

//     /**
//      * Update download state
//      */
//     paybee_kisscam_Data.MemSlotStatus.Entry[DownLoadInfo.MEM].MemoryState = paybee_kisscam_DOWNLOAD_ON_GOING;

//     /**
//      * Inspection
//      */
//     paybee_kisscam_Inspection(DownLoadInfo.MEM);

//     /**
//      * Store Table State to File
//      */
//     Status = paybee_kisscam_WriteToFile(paybee_kisscam_Data.TblHandle, &paybee_kisscam_Data.MemSlotStatus, sizeof(paybee_kisscam_Memory_Status_t), true);
//     if (Status != CFE_SUCCESS) {
//         ErrCnt ++;
//         // PAYBEE_KISSCAM_APP_printf("Write Fail!.\n");
//         goto report;
//     }

//     paybee_kisscam_Data.ErrCount = ErrCnt;

// report: 
//         /* Report */
//     {
//         paybee_kisscam_ReportTlm_t Report = {0, };
//         CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(paybee_kisscam_REPORT_TLM_MID),
//                         sizeof(paybee_kisscam_ReportTlm_t));
//         Report.Report.MsgID = paybee_kisscam_CMD_MID;
//         Report.Report.CommandCode = paybee_kisscam_DOWNLOAD_ALL_CHILD_CC;
//         Report.Report.ReturnType = (paybee_kisscam_Data.ErrCount == 0) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
//         Report.Report.ReturnCode = Status;
//         Report.Report.ReturnDataSize = sizeof(paybee_kisscam_Data.ErrCount);
//         memcpy(Report.Report.ReturnValue, &paybee_kisscam_Data.ErrCount, sizeof(paybee_kisscam_Data.ErrCount));
//         CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
//         CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);
//     }
//     return;
// }


/*************************************************
 * 
 * Transaction function
 * Sequencial read
 * 
 **************************************************/
paybee_kisscam_TransactionResult_t paybee_kisscam_Transaction(const void *Tx, void *Rx,
                                                              size_t RxCapacity, uint8_t CC) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    size_t ReadByte = 0;
    paybee_kisscam_TransactionResult_t Result = {
        .ReturnCode = CFE_SUCCESS,
        .ReturnType = RPT_RETTYPE_SUCCESS,
        .ReadSize = 0
    };

    if (Tx == NULL || Rx == NULL || RxCapacity < paybee_kisscam_HDR_TAIL_SIZE) {
        Result.ReturnCode = CFE_SRL_BAD_ARGUMENT;
        Result.ReturnType = RPT_RETTYPE_APP;
        return Result;
    }

#define PAYBEE_KISSCAM_SERIAL_FAILURE(Code)                                           \
    do {                                                                               \
        if (paybee_kisscam_Data.Handle != NULL) {                                       \
            tcflush(paybee_kisscam_Data.Handle->FD, TCIFLUSH);                          \
        }                                                                              \
        Result.ReturnCode = (Code);                                                     \
        Result.ReturnType = RPT_RETTYPE_CFE;                                            \
        Result.ReadSize = (ReadByte > RxCapacity) ? RxCapacity : ReadByte;              \
        paybee_kisscam_Data.ErrCounter++;                                               \
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,    \
                          "KissCAM Serial Error: CC=0x%02X Status=%ld Read=%lu",        \
                          CC, (long)(Code), (unsigned long)ReadByte);                    \
        return Result;                                                                  \
    } while (0)

    Params.TxData = (void *)Tx;
    Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    Params.RxData = Rx;
    Params.RxSize = 3; // Read the first three response bytes
    Params.Timeout = 1000;

    // Read the first three response bytes
    Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    if (Params.ReadBytes > 0) ReadByte += (size_t)Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        PAYBEE_KISSCAM_SERIAL_FAILURE(Status);
    }
    if (Params.ReadBytes != 3) {
        PAYBEE_KISSCAM_SERIAL_FAILURE(CFE_SRL_PARTIAL_READ_ERR);
    }
    /* Read Tlm payload length */
    memset(&Params, 0, sizeof(Params));
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = (uint8_t *)Rx + 3;
    Params.RxSize = 2; // MSB + LSB
    Params.Timeout = 100;
    
    Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    if (Params.ReadBytes > 0) ReadByte += (size_t)Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        PAYBEE_KISSCAM_SERIAL_FAILURE(Status);
    }
    if (Params.ReadBytes != 2) {
        PAYBEE_KISSCAM_SERIAL_FAILURE(CFE_SRL_PARTIAL_READ_ERR);
    }
    uint16_t Len = ((uint8_t *)Params.RxData)[0] << 8 | ((uint8_t *)Params.RxData)[1];
    size_t TotalSize = paybee_kisscam_HDR_TAIL_SIZE + (size_t)Len;

    if (TotalSize > RxCapacity) {
        Result.ReturnCode = CFE_STATUS_RANGE_ERROR;
        Result.ReturnType = RPT_RETTYPE_APP;
        Result.ReadSize = ReadByte;
        paybee_kisscam_Data.ErrCounter++;
        CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "KissCAM response exceeds buffer: CC=0x%02X Size=%lu Capacity=%lu",
                          CC, (unsigned long)TotalSize, (unsigned long)RxCapacity);
        return Result;
    }

    /* Read Tlm Payload length */
    memset(&Params, 0, sizeof(Params));
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = (uint8_t *)Rx + 5;
    Params.RxSize = Len + 1; // Include Terminate byte
    Params.Timeout = 700;
    if (CC == paybee_kisscam_DOWNLOAD_ALL_CC || CC == paybee_kisscam_DOWNLOAD_CC) {
        Params.Interval = 1000 * 70;
    }
    Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    if (Params.ReadBytes > 0) ReadByte += (size_t)Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        PAYBEE_KISSCAM_SERIAL_FAILURE(Status);
    }
    if (Params.ReadBytes != (ssize_t)(Len + 1)) {
        PAYBEE_KISSCAM_SERIAL_FAILURE(CFE_SRL_PARTIAL_READ_ERR);
    }
    Result.ReadSize = ReadByte;
    CFE_EVS_SendEvent(paybee_kisscam_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "KissCAM response received: CC=0x%02X Read=%lu", CC,
                      (unsigned long)ReadByte);

    return Result;

#undef PAYBEE_KISSCAM_SERIAL_FAILURE
}

/*************************************************
 * 
 * Transaction function without report
 * This should be used in beacon sequence
 * Sequencial read
 * 
 **************************************************/
// void paybee_kisscam_TransactionWithoutReport(void *Tx, void *Rx, uint8_t CC) {
//     int32 Status;
//     CFE_SRL_IO_Param_t Params = {0,};
//     ssize_t ReadByte = 0;

//     Params.TxData = Tx;
//     Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
//     Params.RxData = Rx;
//     Params.RxSize = 3; // Read Start byte, Ack, Mode
//     Params.Timeout = 1000;

//     // Read Start byte, Ack, Mode
//     Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
//     ReadByte += Params.ReadBytes;
//     if (Status != CFE_SUCCESS) {
//         return;
//     }

//     if (((uint8_t *)Rx)[1] == paybee_kisscam_TLM_ERR_FLAG) {
//         return;
//     }

//     /* Read Tlm payload length */
//     memset(&Params, 0, sizeof(Params));
//     Params.TxData = NULL;
//     Params.TxSize = 0;
//     Params.RxData = (uint8_t *)Rx + 3;
//     Params.RxSize = 2; // MSB + LSB
//     Params.Timeout = 100;
//     Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
//     ReadByte += Params.ReadBytes;
//     if (Status != CFE_SUCCESS) {
//         return;
//     }
//     uint16_t Len = ((uint8_t *)Params.RxData)[0] << 8 | ((uint8_t *)Params.RxData)[1];

//     /* Read Tlm Payload length */
//     memset(&Params, 0, sizeof(Params));
//     Params.TxData = NULL;
//     Params.TxSize = 0;
//     Params.RxData = (uint8_t *)Rx + 5;
//     Params.RxSize = Len + 1; // Include Terminate byte
//     Params.Timeout = 700;
//     if (CC == paybee_kisscam_DOWNLOAD_ALL_CC || CC == paybee_kisscam_DOWNLOAD_CC) {
//         Params.Interval = 1000 * 70;
//     }
//     Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
//     if (Status != CFE_SUCCESS) {
//         return;
//     }

//     return;
// }
