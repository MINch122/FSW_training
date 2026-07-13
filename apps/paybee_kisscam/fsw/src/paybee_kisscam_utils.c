#include "paybee_kisscam_internal_cfg.h"
#include "paybee_kisscam_interface_cfg.h"
#include "paybee_kisscam_tblstruct.h"
#include "paybee_kisscam_eventids.h"
#include "paybee_kisscam_task.h"

#include <fcntl.h>
#include <unistd.h>
#include "rpt_interface_cfg.h"

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

/**************************************************
 * Serial Comm success, but H/W error packet came.
 **************************************************/
void paybee_kisscam_HandleErrorPacket(void *ErrPkt, ssize_t Size, uint8_t CC) {
    int32 Status;

    if (ErrPkt == NULL || Size <= 0 || Size > paybee_kisscam_ERROR_TLM_SIZE) return;
    
    if (Size < paybee_kisscam_ERROR_TLM_SIZE) {
        // Read residual data
        uint8_t Residual = paybee_kisscam_ERROR_TLM_SIZE - Size;
        CFE_SRL_IO_Param_t Params = {0,};
        Params.TxData = NULL;
        Params.TxSize = 0;
        Params.RxData = (uint8_t *)ErrPkt + Size;
        Params.RxSize = Residual;
        Params.Timeout = 100;

        Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
        if (Status != CFE_SUCCESS || Params.ReadBytes != Residual) {
            paybee_kisscam_Data.DeviceErrCounter ++;
            paybee_kisscam_Data.ErrCounter ++;
            return;
        }
    }

    // uint8_t MD, CMD, ERR, RXF;
    
    // if (((uint8_t *)ErrPkt)[0] != '@' || ((uint8_t *)ErrPkt)[8] != '\r') return;
    
    // MD  = ((uint8_t *)ErrPkt)[2];
    // CMD = ((uint8_t *)ErrPkt)[5];
    // ERR = ((uint8_t *)ErrPkt)[6];
    // RXF = ((uint8_t *)ErrPkt)[7];
    // PAYBEE_KISSCAM_APP_printf("MD: 0x%02X CMD: 0x%02X ERR: 0x%02X RXF: 0x%02X\n", MD, CMD, ERR, RXF);

    /**
     * Configure Report for RPT
     */
    paybee_kisscam_ReportTlm_t *BufPtr = (paybee_kisscam_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(paybee_kisscam_ReportTlm_t));
    if (BufPtr == NULL) return;

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
        CFE_SB_ValueToMsgId(paybee_kisscam_REPORT_TLM_MID),
        sizeof(paybee_kisscam_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    BufPtr->Report.MsgID = paybee_kisscam_CMD_MID;
    BufPtr->Report.CommandCode = CC;
    BufPtr->Report.ReturnType = RPT_RETTYPE_HW;
    BufPtr->Report.ReturnCode = 0x23;
    BufPtr->Report.ReturnDataSize = paybee_kisscam_ERROR_TLM_SIZE;
    memcpy(BufPtr->Report.ReturnValue, ErrPkt, paybee_kisscam_ERROR_TLM_SIZE);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
    }
    
    CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                      "KissCAM HW Error: CC = 0x%02X", CC);
                      
    PAYBEE_KISSCAM_APP_printf("[KissCAM] HW Error Packet Received! CC: 0x%02X\n", CC);

    return;
}

/************************************************
 * Serial Comm. failed
 ************************************************/
void paybee_kisscam_HandleErrorSerial(int32 Status, uint8 CC, void *ReadData, ssize_t ReadSize) {
    
    paybee_kisscam_Data.ErrCounter ++;

    paybee_kisscam_ReportTlm_t *BufPtr = (paybee_kisscam_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(paybee_kisscam_ReportTlm_t));
    if (BufPtr == NULL) return;

    if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
        CFE_SB_ValueToMsgId(paybee_kisscam_REPORT_TLM_MID),
        sizeof(paybee_kisscam_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    BufPtr->Report.MsgID = paybee_kisscam_CMD_MID;
    BufPtr->Report.CommandCode = CC;
    BufPtr->Report.ReturnType = RPT_RETTYPE_CFE;
    BufPtr->Report.ReturnCode = Status;
    BufPtr->Report.ReturnDataSize = (uint16_t)ReadSize;
    memcpy(BufPtr->Report.ReturnValue, ReadData, 
            ReadSize > sizeof(BufPtr->Report.ReturnValue) ? sizeof(BufPtr->Report.ReturnValue) : ReadSize);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
    }
    
    CFE_EVS_SendEvent(paybee_kisscam_CMD_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                      "KissCAM Serial Error: CC = 0x%02X, Status = %d", CC, Status);
                      
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Serial Comm Error! CC: 0x%02X, Status: %d\n", CC, Status);

    return;
}

/*************************************************
 * All success - Report to RPT
 * **Special case**
 * Noop cmd report the **CmdCounter & ErrCounter**
 ************************************************/
void paybee_kisscam_HandleSuccess(uint8_t CC, void *ReadData, ssize_t ReadSize) {
    paybee_kisscam_ReportTlm_t *BufPtr = (paybee_kisscam_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(paybee_kisscam_ReportTlm_t));
    if (BufPtr == NULL) return;

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(paybee_kisscam_REPORT_TLM_MID),
        sizeof(paybee_kisscam_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    BufPtr->Report.MsgID = paybee_kisscam_CMD_MID;
    BufPtr->Report.CommandCode = CC;
    BufPtr->Report.ReturnType = RPT_RETTYPE_SUCCESS;
    BufPtr->Report.ReturnCode = CFE_SUCCESS;
    BufPtr->Report.ReturnDataSize = (uint16_t)ReadSize;
    memcpy(BufPtr->Report.ReturnValue, ReadData, 
            ReadSize > sizeof(BufPtr->Report.ReturnValue) ? sizeof(BufPtr->Report.ReturnValue) : ReadSize);
    
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
    }
    
    /* 지상국에 텍스트 로그 형태로 성공했음을 알림 */
    CFE_EVS_SendEvent(paybee_kisscam_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "KissCAM Command Success: CC = 0x%02X", CC);
                      
    PAYBEE_KISSCAM_APP_printf("[KissCAM] Command Success! CC: 0x%02X\n", CC);

    return;
}

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
void paybee_kisscam_Transaction(void *Tx, void *Rx, uint8_t CC) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    ssize_t ReadByte = 0;

    Params.TxData = Tx;
    Params.TxSize = paybee_kisscam_CMD_PKT_SIZE;
    Params.RxData = Rx;
    Params.RxSize = 3; // Read Start byte, Ack, Mode
    Params.Timeout = 1000;

    // Read Start byte, Ack, Mode
    Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    ReadByte += Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        paybee_kisscam_HandleErrorSerial(Status, CC, Rx, ReadByte);
        return;
    }

    if (((uint8_t *)Rx)[1] == paybee_kisscam_TLM_ERR_FLAG) {
        paybee_kisscam_HandleErrorPacket(Rx, ReadByte, CC);
        return;
    }

    /* Read Tlm payload length */
    memset(&Params, 0, sizeof(Params));
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = (uint8_t *)Rx + 3;
    Params.RxSize = 2; // MSB + LSB
    Params.Timeout = 100;
    
    Status = CFE_SRL_ApiRead(paybee_kisscam_Data.Handle, &Params);
    ReadByte += Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        paybee_kisscam_HandleErrorSerial(Status, CC, Rx, Params.ReadBytes);
        return;
    }
    uint16_t Len = ((uint8_t *)Params.RxData)[0] << 8 | ((uint8_t *)Params.RxData)[1];

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
    if (Status != CFE_SUCCESS) {
        paybee_kisscam_HandleErrorSerial(Status, CC, Rx, Params.ReadBytes);
        return;
    }

    paybee_kisscam_HandleSuccess(CC, Rx, paybee_kisscam_HDR_TAIL_SIZE + Len);

    return;
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
