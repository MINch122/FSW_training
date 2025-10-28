#include "payuzuc_internal_cfg.h"
#include "payuzuc_interface_cfg.h"
#include "payuzuc_tblstruct.h"

#include "payuzuc_task.h"

#include <fcntl.h>
#include <unistd.h>
#include "rpt_interface_cfg.h"

void PAYUZUC_SetLineTrue(uint8_t MemSlot, uint16_t Line) {
    if(PAYUZUC_Data.MemSlotStatus.Entry[MemSlot].MemoryState == PAYUZUC_DOWNLOAD_DONE) return;
    
    PAYUZUC_Data.MemSlotStatus.Entry[MemSlot].LineState[Line / 8] |= (1 << (Line % 8));

    return;
}


int PAYUZUC_OpenTblFile(void) {
    // int32 Status;
    int ID;

    ID = open(PAYUZUC_TBL_PATH, O_CREAT | O_RDWR, 0666);
    OS_printf("ID: %d\n",ID);
    return ID;
}


int PAYUZUC_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum) {
    int ID;
    char Path[64] = {0, };

    /**
     * One line Download
     */
    if (LineNum == 0) {
        sprintf(Path, "%s%u_%u_%03u", PAYUZUC_IMG_PATH, MemorySlot, 
            PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine);    
    }
    else sprintf(Path, "%s%u_%u_%03u-%03u", PAYUZUC_IMG_PATH, MemorySlot, 
                PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine, StartLine + LineNum - 1);
    
    OS_printf("Path: %s\n", Path);
    
    ID = open(Path, O_CREAT| O_TRUNC | O_WRONLY, 0666);
    return ID;
}


int32 PAYUZUC_WriteToFile(int ID, void *Data, size_t Size, bool IsTbl) {
    int32 Status;
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


int32 PAYUZUC_ReadFile(int ID, void *Data, size_t Size) {
    return read(ID, Data, Size);
}


int32 PAYUZUC_CloseFile(int ID) {
    return close(ID);
}


void PAYUZUC_Inspection(uint8_t MemorySlot) {
    for (uint8_t i = 0; i < 60; i++) {
        if (PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] != 0xFF) {
            return;
        }
    }
    OS_printf("Memory Slot %u Download Done.\n", MemorySlot);
    PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].MemoryState = PAYUZUC_DOWNLOAD_DONE;
    PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx ++;
    for (uint8_t i = 0; i < 60; i++) {
        PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] = 0;
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
void PAYUZUC_HandleErrorPacket(void *ErrPkt, ssize_t Size, uint8_t CC) {
    int32 Status;

    if (ErrPkt == NULL || Size <= 0 || Size > PAYUZUC_ERROR_TLM_SIZE) return;
    
    if (Size < PAYUZUC_ERROR_TLM_SIZE) {
        // Read residual data
        uint8_t Residual = PAYUZUC_ERROR_TLM_SIZE - Size;
        CFE_SRL_IO_Param_t Params = {0,};
        Params.TxData = NULL;
        Params.TxSize = 0;
        Params.RxData = (uint8_t *)ErrPkt + Size;
        Params.RxSize = Residual;
        Params.Timeout = 100;

        Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
        if (Status != CFE_SUCCESS || Params.ReadBytes != Residual) {
            PAYUZUC_Data.DeviceErrCounter ++;
            PAYUZUC_Data.ErrCounter ++;
            return;
        }
    }

    uint8_t MD, CMD, ERR, RXF;
    
    if (((uint8_t *)ErrPkt)[0] != '@' || ((uint8_t *)ErrPkt)[8] != '\r') return;
    
    MD  = ((uint8_t *)ErrPkt)[2];
    CMD = ((uint8_t *)ErrPkt)[5];
    ERR = ((uint8_t *)ErrPkt)[6];
    RXF = ((uint8_t *)ErrPkt)[7];
    OS_printf("MD: 0x%02X CMD: 0x%02X ERR: 0x%02X RXF: 0x%02X\n", MD, CMD, ERR, RXF);

    /**
     * Configure Report for RPT
     */
    PAYUZUC_ReportTlm_t *BufPtr = (PAYUZUC_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUC_ReportTlm_t));
    if (BufPtr == NULL) return;

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
        CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID),
        sizeof(PAYUZUC_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    BufPtr->Report.MsgID = PAYUZUC_CMD_MID;
    BufPtr->Report.CommandCode = CC;
    BufPtr->Report.ReturnType = RPT_RETTYPE_HW;
    BufPtr->Report.ReturnCode = 0x23;
    BufPtr->Report.ReturnDataSize = PAYUZUC_ERROR_TLM_SIZE;
    memcpy(BufPtr->Report.ReturnValue, ErrPkt, PAYUZUC_ERROR_TLM_SIZE);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
    }

    return;
}

/************************************************
 * Serial Comm. failed
 ************************************************/
void PAYUZUC_HandleErrorSerial(int32 Status, uint8 CC, void *ReadData, ssize_t ReadSize) {
    
    PAYUZUC_Data.ErrCounter ++;

    PAYUZUC_ReportTlm_t *BufPtr = (PAYUZUC_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUC_ReportTlm_t));
    if (BufPtr == NULL) return;

    if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
        CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID),
        sizeof(PAYUZUC_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    BufPtr->Report.MsgID = PAYUZUC_CMD_MID;
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

    return;
}

/*************************************************
 * All success - Report to RPT
 * **Special case**
 * Noop cmd report the **CmdCounter & ErrCounter**
 ************************************************/
void PAYUZUC_HandleSuccess(uint8_t CC, void *ReadData, ssize_t ReadSize) {
    PAYUZUC_ReportTlm_t *BufPtr = (PAYUZUC_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUC_ReportTlm_t));
    if (BufPtr == NULL) return;

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID),
        sizeof(PAYUZUC_ReportTlm_t)) != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    BufPtr->Report.MsgID = PAYUZUC_CMD_MID;
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

    return;
}

/***********************************************
 * 
 * Packet Configuration Util func
 * 
 ***********************************************/
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

/***********************************************
 * 
 * Download task util function
 * @deprecated not used
 * 
 ***********************************************/
void PAYUZUC_DownloadTask(void) {

    OS_MutSemTake(PAYUZUC_Data.MutId);
    PAYUZUC_DownloadAll_Payload_t DownLoadInfo = PAYUZUC_Data.DownTaskArg;
    OS_MutSemGive(PAYUZUC_Data.MutId);

    int32 Status;
    PAYUZUC_Cmd_t Cmd = {0,};
    uint8_t RxBuf[PAYUZUC_DOWNLOAD_TLM_SIZE] = {0};
    uint16_t ErrCnt = 0;

    const uint16_t TotLine = DownLoadInfo.PRE ? PAYUZUC_THUMBNAIL_IMG_LINE_NUM : PAYUZUC_IMG_LINE_NUM;;

    /* Validate the args */
    if (DownLoadInfo.PRE != PAYUZUC_DOWNLOAD_THUMBNAIL_FLAG && DownLoadInfo.PRE != PAYUZUC_DOWNLOAD_ORIGINAL_FLAG) {
        PAYUZUC_Data.ErrCounter ++;
        Status = CFE_STATUS_VALIDATION_FAILURE;
        goto report;
    }

    if (DownLoadInfo.StartLine < 0 || (DownLoadInfo.StartLine + DownLoadInfo.LineNum) > TotLine) {
        PAYUZUC_Data.ErrCounter ++;
        Status = CFE_STATUS_VALIDATION_FAILURE;
        goto report;
    }

    int FD = PAYUZUC_OpenFile(DownLoadInfo.MEM, DownLoadInfo.StartLine, DownLoadInfo.LineNum);

    uint8_t Payload[PAYUZUC_DOWNLOAD_PARAM_SIZE] = {0,};
    Payload[0] = DownLoadInfo.MEM;
    Payload[1] = DownLoadInfo.PRE;

    for (uint16_t line = DownLoadInfo.StartLine; line < (DownLoadInfo.StartLine + DownLoadInfo.LineNum); 
        line ++) {
        Payload[2] = (line >> 8) & 0xFF; // Line MSB
        Payload[3] = line & 0xFF;       // Line LSB
        PAYUZUC_ConfigurePacket(Payload, &Cmd, PAYUZUC_DOWNLOAD_PARAM_SIZE,
                            PAYUZUC_DOWNLOAD_CMD_CODE);
        
        CFE_SRL_IO_Param_t Params = {0,};
        Params.TxData = &Cmd;
        Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
        Params.RxData = &RxBuf;
        Params.RxSize = DownLoadInfo.PRE ? PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE : PAYUZUC_DOWNLOAD_TLM_SIZE;
        Params.Timeout = 200;
        Params.Interval = 80; // Empirical value 70ms, Margin for stability

        Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
        if (Status != CFE_SUCCESS) {
            ErrCnt ++;
            PAYUZUC_HandleErrorSerial(Status, PAYUZUC_DOWNLOAD_ALL_CC, Params.RxData, Params.ReadBytes);
            continue;
        }
        else if (RxBuf[1] == PAYUZUC_TLM_ERR_FLAG) {
            PAYUZUC_HandleErrorPacket(RxBuf, Params.ReadBytes, PAYUZUC_DOWNLOAD_ALL_CC);
            ErrCnt ++;
            PAYUZUC_Data.DeviceErrCounter ++;
            break;
        }
        
        /**
         * Write Image Data to file
         */
        Status  = PAYUZUC_WriteToFile(FD, RxBuf, DownLoadInfo.PRE ? PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE : PAYUZUC_DOWNLOAD_TLM_SIZE, false);
        if (Status != CFE_SUCCESS) {
            OS_printf("Write Error. RC = %d\n", Status);
            ErrCnt ++;
            continue;
        }
        memset(RxBuf, 0, sizeof(RxBuf));

        /**
         * Update line state
         */
        PAYUZUC_SetLineTrue(DownLoadInfo.MEM, line);

        /* Debugging */
        OS_printf("%s:Line %u Download done.\n", __func__, line);

    }
    /**
     * When Download Done, Close file
     */
    Status = PAYUZUC_CloseFile(FD);
    if (Status != CFE_SUCCESS) {
        ErrCnt ++;
        PAYUZUC_Data.ErrCounter ++;
        goto report;
    }

    /**
     * Update download state
     */
    PAYUZUC_Data.MemSlotStatus.Entry[DownLoadInfo.MEM].MemoryState = PAYUZUC_DOWNLOAD_ON_GOING;

    /**
     * Inspection
     */
    PAYUZUC_Inspection(DownLoadInfo.MEM);

    /**
     * Store Table State to File
     */
    Status = PAYUZUC_WriteToFile(PAYUZUC_Data.TblHandle, &PAYUZUC_Data.MemSlotStatus, sizeof(PAYUZUC_Memory_Status_t), true);
    if (Status != CFE_SUCCESS) {
        ErrCnt ++;
        OS_printf("Write Fail!.\n");
        goto report;
    }

    PAYUZUC_Data.ErrCount = ErrCnt;

report: 
        /* Report */
    {
        PAYUZUC_ReportTlm_t Report = {0, };
        CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID),
                        sizeof(PAYUZUC_ReportTlm_t));
        Report.Report.MsgID = PAYUZUC_CMD_MID;
        Report.Report.CommandCode = PAYUZUC_DOWNLOAD_ALL_CHILD_CC;
        Report.Report.ReturnType = (PAYUZUC_Data.ErrCount == 0) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
        Report.Report.ReturnCode = Status;
        Report.Report.ReturnDataSize = sizeof(PAYUZUC_Data.ErrCount);
        memcpy(Report.Report.ReturnValue, &PAYUZUC_Data.ErrCount, sizeof(PAYUZUC_Data.ErrCount));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);
    }
    return;
}


/*************************************************
 * 
 * Transaction function
 * Sequencial read
 * 
 **************************************************/
void PAYUZUC_Transaction(void *Tx, void *Rx, uint8_t CC) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    ssize_t ReadByte = 0;

    Params.TxData = Tx;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = Rx;
    Params.RxSize = 3; // Read Start byte, Ack, Mode
    Params.Timeout = 1000;

    // Read Start byte, Ack, Mode
    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    ReadByte += Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        PAYUZUC_HandleErrorSerial(Status, CC, Rx, ReadByte);
        return;
    }

    if (((uint8_t *)Rx)[1] == PAYUZUC_TLM_ERR_FLAG) {
        PAYUZUC_HandleErrorPacket(Rx, ReadByte, CC);
        return;
    }

    /* Read Tlm payload length */
    memset(&Params, 0, sizeof(Params));
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = (uint8_t *)Rx + 3;
    Params.RxSize = 2; // MSB + LSB
    Params.Timeout = 100;
    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    ReadByte += Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        PAYUZUC_HandleErrorSerial(Status, CC, Rx, Params.ReadBytes);
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
    if (CC == PAYUZUC_DOWNLOAD_ALL_CC || CC == PAYUZUC_DOWNLOAD_CC) {
        Params.Interval = 1000 * 70;
    }
    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        PAYUZUC_HandleErrorSerial(Status, CC, Rx, Params.ReadBytes);
        return;
    }

    PAYUZUC_HandleSuccess(CC, Rx, PAYUZUC_HDR_TAIL_SIZE + Len);

    return;
}

/*************************************************
 * 
 * Transaction function without report
 * This should be used in beacon sequence
 * Sequencial read
 * 
 **************************************************/
void PAYUZUC_TransactionWithoutReport(void *Tx, void *Rx, uint8_t CC) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};
    ssize_t ReadByte = 0;

    Params.TxData = Tx;
    Params.TxSize = PAYUZUC_CMD_PKT_SIZE;
    Params.RxData = Rx;
    Params.RxSize = 3; // Read Start byte, Ack, Mode
    Params.Timeout = 1000;

    // Read Start byte, Ack, Mode
    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    ReadByte += Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
        return;
    }

    if (((uint8_t *)Rx)[1] == PAYUZUC_TLM_ERR_FLAG) {
        return;
    }

    /* Read Tlm payload length */
    memset(&Params, 0, sizeof(Params));
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = (uint8_t *)Rx + 3;
    Params.RxSize = 2; // MSB + LSB
    Params.Timeout = 100;
    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    ReadByte += Params.ReadBytes;
    if (Status != CFE_SUCCESS) {
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
    if (CC == PAYUZUC_DOWNLOAD_ALL_CC || CC == PAYUZUC_DOWNLOAD_CC) {
        Params.Interval = 1000 * 70;
    }
    Status = CFE_SRL_ApiRead(PAYUZUC_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) {
        return;
    }

    return;
}