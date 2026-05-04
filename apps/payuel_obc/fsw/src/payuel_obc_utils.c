#include "osapi.h"
#include "payuel_obc.h"
#include "payuel_obc_eventids.h"
#include "payuel_obc_tbl.h"
#include "payuel_obc_utils.h"
#include "payuel_obc_mission_cfg.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/*
 * Checksum helpers used by the OBC-side payload protocol.
 * CRC16 protects command/response frames. CRC32 is used for reconstructed
 * image and BIN files whose expected CRC is reported by metadata.
 */
static uint16_t PAYUEL_OBC_Crc16(const uint8_t *Data, size_t Length)
{
    uint16_t Crc = 0xFFFF;
    size_t   Index;
    int      Bit;

    for (Index = 0; Index < Length; ++Index)
    {
        /* Mix the next byte into the upper half of the register first. */
        Crc ^= (uint16_t)Data[Index] << 8;
        for (Bit = 0; Bit < 8; ++Bit)
        {
            /* Apply the CCITT polynomial one bit at a time. */
            if ((Crc & 0x8000U) != 0U)
            {
                Crc = (uint16_t)((Crc << 1) ^ 0x1021U);
            }
            else
            {
                Crc <<= 1;
            }
        }
    }

    return Crc;
}

static uint32_t PAYUEL_OBC_Crc32Update(uint32_t Crc, const uint8_t *Data, size_t Length)
{
    size_t   Index;
    int      Bit;

    for (Index = 0; Index < Length; ++Index)
    {
        /* CRC32 is updated incrementally so callers can feed one buffer at a time. */
        Crc ^= Data[Index];
        for (Bit = 0; Bit < 8; ++Bit)
        {
            /* Use the reflected CRC32 polynomial expected by the payload/file format. */
            if ((Crc & 1U) != 0U)
            {
                Crc = (Crc >> 1) ^ 0xEDB88320u;
            }
            else
            {
                Crc >>= 1;
            }
        }
    }

    return Crc;
}

/* These helpers translate between raw wire bytes and host integer types. */
uint16_t PAYUEL_OBC_ReadU16BE(const uint8_t *Data)
{
    return (uint16_t)(((uint16_t)Data[0] << 8) | (uint16_t)Data[1]);
}

uint32_t PAYUEL_OBC_ReadU32BE(const uint8_t *Data)
{
    return ((uint32_t)Data[0] << 24) | ((uint32_t)Data[1] << 16) |
           ((uint32_t)Data[2] << 8)  |  (uint32_t)Data[3];
}

void PAYUEL_OBC_WriteU16BE(uint8_t *Data, uint16_t Value)
{
    Data[0] = (uint8_t)(Value >> 8);
    Data[1] = (uint8_t)(Value & 0xFFU);
}

void PAYUEL_OBC_WriteU32BE(uint8_t *Data, uint32_t Value)
{
    Data[0] = (uint8_t)(Value >> 24);
    Data[1] = (uint8_t)(Value >> 16);
    Data[2] = (uint8_t)(Value >> 8);
    Data[3] = (uint8_t)(Value & 0xFFU);
}

bool PAYUEL_OBC_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              Result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    /* cFS command handlers validate the incoming packet size before parsing fields. */
    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength)
    {
        /* Include ID and function code so the bad sender can be identified quickly. */
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(PAYUEL_OBC_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID=0x%X CC=%u Len=%u Expected=%u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId),
                          (unsigned int)FcnCode,
                          (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        Result = false;
        PAYUEL_OBC_Data.ErrCounter++;
    }

    return Result;
}

void PAYUEL_OBC_AppendCrc16(uint8_t *Buf, size_t PayloadLength)
{
    uint16_t Crc = PAYUEL_OBC_Crc16(Buf, PayloadLength);

    /* The CRC is appended immediately after the payload bytes. */
    PAYUEL_OBC_WriteU16BE(&Buf[PayloadLength], Crc);
}

bool PAYUEL_OBC_VerifyCrc16(const uint8_t *Data, size_t TotalLength)
{
    size_t   PayloadLength;
    uint16_t Computed;
    uint16_t Received;

    if (TotalLength < 3U)
    {
        /* Need at least 1 byte of payload and 2 bytes of CRC. */
        return false;
    }

    /* Split the buffer into [payload][crc16] and compare both values. */
    PayloadLength = TotalLength - 2U;
    Computed      = PAYUEL_OBC_Crc16(Data, PayloadLength);
    Received      = PAYUEL_OBC_ReadU16BE(&Data[PayloadLength]);

    return (Computed == Received);
}

static bool PAYUEL_OBC_IsPayloadErrorPacket(uint8_t ExpectedCmd, const uint8_t *Data, int32 Length)
{
    return (Data != NULL &&
            Length == 4 &&
            Data[0] == ExpectedCmd &&
            PAYUEL_OBC_VerifyCrc16(Data, 4U));
}

void PAYUEL_OBC_ClearImageMetaCache(void)
{
    /* Reset cached image metadata so the next request starts from known state. */
    PAYUEL_OBC_Data.ImageMetaSlot      = 0U;
    PAYUEL_OBC_Data.ImageMetaNumber    = 0U;
    PAYUEL_OBC_Data.ImageTotalChunks   = 0U;
    PAYUEL_OBC_Data.ImageLastChunkSize = 0U;
    PAYUEL_OBC_Data.ImageFileCrc32     = 0U;
}

void PAYUEL_OBC_ClearSensorMetaCache(void)
{
    /* Reset cached sensor metadata independently from the image cache. */
    PAYUEL_OBC_Data.SensorMetaSlot      = 0U;
    PAYUEL_OBC_Data.SensorMetaNumber    = 0U;
    PAYUEL_OBC_Data.SensorTotalChunks   = 0U;
    PAYUEL_OBC_Data.SensorLastChunkSize = 0U;
    PAYUEL_OBC_Data.SensorFileCrc32     = 0U;
}

void PAYUEL_OBC_ReportCmdStatus(CFE_SB_MsgId_t MsgId, uint16 CommandCode, int32 Status,
                                const uint8_t *Data, uint16 DataLength, uint8 ReturnType)
{
    RPT_Report_t Report = (RPT_Report_t){0};
    int32        OsStatus;
    bool         ReportLocked = false;

    /* The RPT buffer is stored in app-global state, so concurrent writers must serialize access. */
    if (OS_ObjectIdDefined(PAYUEL_OBC_Data.ReportMutex))
    {
        /* OBC checks that the mutex exists because startup ordering can differ by deployment. */
        OsStatus = OS_MutSemTake(PAYUEL_OBC_Data.ReportMutex);
        if (OsStatus != OS_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Report mutex take failed, RC=%ld", (long)OsStatus);
            PAYUEL_OBC_Data.ErrCounter++;
        }
        else
        {
            ReportLocked = true;
        }
    }

    /* Copy the command context into the report packet seen by the rest of the system. */
    Report.MsgID       = CFE_SB_MsgIdToValue(MsgId);
    Report.CommandCode = CommandCode;
    Report.ReturnType  = ReturnType;
    Report.ReturnCode  = Status;

    if (Data != NULL && DataLength > 0U)
    {
        /* Clamp the returned payload so we never overrun the fixed telemetry field. */
        uint16 CopySize = (DataLength <= sizeof(Report.ReturnValue))
                              ? DataLength
                              : (uint16)sizeof(Report.ReturnValue);

        Report.ReturnDataSize = CopySize;
        memcpy(Report.ReturnValue, Data, CopySize);
    }

    /* Timestamp and publish once the report body is fully populated. */
    PAYUEL_OBC_Data.rpt.Payload = Report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_OBC_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_OBC_Data.rpt.TelemetryHeader), true);

    if (ReportLocked)
    {
        OsStatus = OS_MutSemGive(PAYUEL_OBC_Data.ReportMutex);
        if (OsStatus != OS_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Report mutex give failed, RC=%ld", (long)OsStatus);
            PAYUEL_OBC_Data.ErrCounter++;
        }
    }
}

int32 PAYUEL_OBC_LockHardware(const char *Context)
{
    int32 OsStatus;

    /* One hardware lock keeps normal commands and the child download worker from interleaving CSP/CAN transfers. */
    OsStatus = OS_MutSemTake(PAYUEL_OBC_Data.HardwareMutex);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s hardware mutex take failed, RC=%ld",
                          Context, (long)OsStatus);
        PAYUEL_OBC_Data.ErrCounter++;
    }

    return OsStatus;
}

void PAYUEL_OBC_UnlockHardware(const char *Context)
{
    int32 OsStatus;

    /* Release the shared payload bus after the current request/response sequence fully finishes. */
    OsStatus = OS_MutSemGive(PAYUEL_OBC_Data.HardwareMutex);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s hardware mutex give failed, RC=%ld",
                          Context, (long)OsStatus);
        PAYUEL_OBC_Data.ErrCounter++;
    }
}

static CFE_Status_t PAYUEL_OBC_RejectRxLength(const char *CmdName, int32 ActualLength, size_t ExpectedLength)
{
    CFE_EVS_SendEvent(PAYUEL_OBC_RX_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                      "PAYUEL_OBC: %s RX length mismatch (actual=%ld expected=%u)",
                      CmdName, (long)ActualLength, (unsigned int)ExpectedLength);
    PAYUEL_OBC_Data.ErrCounter++;
    return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
}

CFE_Status_t PAYUEL_OBC_RejectHwStatus(const char *CmdName, uint8_t HwStatus)
{
    CFE_EVS_SendEvent(PAYUEL_OBC_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                      "PAYUEL_OBC: %s payload/HW status error 0x%02X", CmdName, HwStatus);
    PAYUEL_OBC_Data.ErrCounter++;
    return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
}

CFE_Status_t PAYUEL_OBC_ValidateResponse(const char *CmdName, uint8_t ExpectedCmd, const uint8_t *RxData,
                                         int32 RspLen, size_t ExpectedLength, uint8_t *ReturnTypeOut,
                                         uint8_t *PayloadErrorPacketOut)
{
    CFE_Status_t Status = CFE_SUCCESS;

    if (PayloadErrorPacketOut != NULL)
    {
        memset(PayloadErrorPacketOut, 0, 4U);
    }

    if (RspLen <= 0)
    {
        Status = (RspLen == 0) ? CFE_STATUS_EXTERNAL_RESOURCE_FAIL : RspLen;
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_CFE;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s CSP error, status=0x%08X", CmdName, Status);
        PAYUEL_OBC_Data.ErrCounter++;
        return Status;
    }

    if (ExpectedLength > 4U && PAYUEL_OBC_IsPayloadErrorPacket(ExpectedCmd, RxData, RspLen))
    {
        if (PayloadErrorPacketOut != NULL)
        {
            memcpy(PayloadErrorPacketOut, RxData, 4U);
        }
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectHwStatus(CmdName, RxData[1]);
    }

    if ((size_t)RspLen != ExpectedLength)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectRxLength(CmdName, RspLen, ExpectedLength);
    }

    if (RxData[0] != ExpectedCmd || !PAYUEL_OBC_VerifyCrc16(RxData, (size_t)RspLen))
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_CRC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s RX CRC/CMD mismatch", CmdName);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

static CFE_Status_t PAYUEL_OBC_ValidateChunkResponse(const char *CmdName, uint8_t ExpectedCmd,
                                                     const uint8_t *RxData, int32 RspLen,
                                                     size_t ExpectedLength,
                                                     uint8_t *ReturnTypeOut,
                                                     uint8_t *PayloadErrorPacketOut)
{
    CFE_Status_t Status = CFE_SUCCESS;

    if (PayloadErrorPacketOut != NULL)
    {
        memset(PayloadErrorPacketOut, 0, 4U);
    }

    if (RspLen <= 0)
    {
        Status = (RspLen == 0) ? CFE_STATUS_EXTERNAL_RESOURCE_FAIL : RspLen;
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_CFE;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s CSP error, status=0x%08X", CmdName, Status);
        PAYUEL_OBC_Data.ErrCounter++;
        return Status;
    }

    if (PAYUEL_OBC_IsPayloadErrorPacket(ExpectedCmd, RxData, RspLen))
    {
        if (PayloadErrorPacketOut != NULL)
        {
            memcpy(PayloadErrorPacketOut, RxData, 4U);
        }
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectHwStatus(CmdName, RxData[1]);
    }

    if ((size_t)RspLen != ExpectedLength)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectRxLength(CmdName, RspLen, ExpectedLength);
    }

    if (RxData[0] != ExpectedCmd || !PAYUEL_OBC_VerifyCrc16(RxData, (size_t)RspLen))
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_CRC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s RX CRC/CMD mismatch", CmdName);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

size_t PAYUEL_OBC_GetChunkDataLenFromImageMeta(const PAYUEL_OBC_ImageMetaInfo_t *Meta, uint16_t ChunkNumber)
{
    /*
     * Most image chunks are fixed-size.
     * Only the last chunk may be shorter, and that length comes from metadata.
     */
    if (Meta != NULL &&
        Meta->ChunkCount > 0U &&
        (uint32_t)ChunkNumber + 1U == Meta->ChunkCount &&
        Meta->LastChunkSize > 0U &&
        Meta->LastChunkSize <= PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        return Meta->LastChunkSize;
    }

    return PAYUEL_OBC_CHUNK_DATA_SIZE;
}

size_t PAYUEL_OBC_GetChunkDataLenFromSensorMeta(const PAYUEL_OBC_SensorMetaInfo_t *Meta, uint16_t ChunkNumber)
{
    /*
     * Sensor downloads use the same chunking rule as image downloads:
     * only the last chunk may be shorter than the nominal chunk size.
     */
    if (Meta != NULL &&
        Meta->ChunkCount > 0U &&
        (uint32_t)ChunkNumber + 1U == Meta->ChunkCount &&
        Meta->LastChunkSize > 0U &&
        Meta->LastChunkSize <= PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        return Meta->LastChunkSize;
    }

    return PAYUEL_OBC_CHUNK_DATA_SIZE;
}

bool PAYUEL_OBC_GetCachedImageMeta(uint8_t CameraID, uint8_t ImageNumber,
                                   PAYUEL_OBC_ImageMetaInfo_t *MetaOut)
{
    /* Reuse metadata from a previous request when it matches the same image selection. */
    if (PAYUEL_OBC_Data.ImageTotalChunks > 0U &&
        PAYUEL_OBC_Data.ImageMetaSlot == CameraID &&
        PAYUEL_OBC_Data.ImageMetaNumber == ImageNumber &&
        PAYUEL_OBC_Data.ImageLastChunkSize > 0U &&
        PAYUEL_OBC_Data.ImageLastChunkSize <= PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        if (MetaOut != NULL)
        {
            MetaOut->Status         = 0U;
            MetaOut->CameraID       = PAYUEL_OBC_Data.ImageMetaSlot;
            MetaOut->ImageValid     = 1U;
            MetaOut->ImageIndex     = PAYUEL_OBC_Data.ImageMetaNumber;
            MetaOut->ImageFileCRC32 = PAYUEL_OBC_Data.ImageFileCrc32;
            MetaOut->LastChunkSize  = PAYUEL_OBC_Data.ImageLastChunkSize;
            MetaOut->ChunkCount     = PAYUEL_OBC_Data.ImageTotalChunks;
        }

        return true;
    }

    return false;
}

bool PAYUEL_OBC_GetCachedSensorMeta(uint8_t DataSlot, uint8_t DataNumber,
                                    PAYUEL_OBC_SensorMetaInfo_t *MetaOut)
{
    /* Reuse metadata from a previous request when it matches the same sensor data selection. */
    if (PAYUEL_OBC_Data.SensorTotalChunks > 0U &&
        PAYUEL_OBC_Data.SensorMetaSlot == DataSlot &&
        PAYUEL_OBC_Data.SensorMetaNumber == DataNumber &&
        PAYUEL_OBC_Data.SensorLastChunkSize > 0U &&
        PAYUEL_OBC_Data.SensorLastChunkSize <= PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        if (MetaOut != NULL)
        {
            MetaOut->Status        = 0U;
            MetaOut->DataSlot      = PAYUEL_OBC_Data.SensorMetaSlot;
            MetaOut->DataIndex     = PAYUEL_OBC_Data.SensorMetaNumber;
            MetaOut->BinValid      = 1U;
            MetaOut->BinSize       = 0U;
            MetaOut->BinFileCRC32  = PAYUEL_OBC_Data.SensorFileCrc32;
            MetaOut->ChunkCount    = PAYUEL_OBC_Data.SensorTotalChunks;
            MetaOut->LastChunkSize = PAYUEL_OBC_Data.SensorLastChunkSize;
        }

        return true;
    }

    return false;
}

CFE_Status_t PAYUEL_OBC_RequestImageMeta(uint8_t CameraID, uint8_t ImageNumber,
                                         PAYUEL_OBC_ImageMetaInfo_t *MetaOut, uint8_t *ReturnTypeOut,
                                         uint8_t *PayloadErrorPacketOut)
{
    uint8_t                 TxData[5];
    uint8_t                 RxData[18] = {0};
    int32                   rx_len;
    CFE_Status_t            status = CFE_SUCCESS;
    PAYUEL_OBC_ImageMetaInfo_t Meta = {0};

    if (ReturnTypeOut != NULL)
    {
        /* Assume success first, then downgrade if any stage fails. */
        *ReturnTypeOut = RPT_RETTYPE_SUCCESS;
    }

    /* Request format: [cmd][camera][image][crc16]. */
    TxData[0] = PAYUEL_OBC_ID_DOWNLOAD_META;
    TxData[1] = CameraID;
    TxData[2] = ImageNumber;
    PAYUEL_OBC_AppendCrc16(TxData, 3);

    /* Send the request and wait for the fixed-length metadata response frame. */
    rx_len = CFE_SRL_ApiTransactionCSP(
        PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
        TxData, sizeof(TxData),
        RxData, sizeof(RxData));

    status = PAYUEL_OBC_ValidateResponse("DownloadMeta", PAYUEL_OBC_ID_DOWNLOAD_META,
                                         RxData, rx_len, sizeof(RxData),
                                         ReturnTypeOut, PayloadErrorPacketOut);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    Meta.Status         = RxData[1];
    Meta.CameraID       = RxData[2];
    Meta.ImageValid     = RxData[3];
    Meta.ImageIndex     = RxData[4];
    Meta.ImageSize      = PAYUEL_OBC_ReadU32BE(&RxData[5]);
    Meta.ImageFileCRC32 = PAYUEL_OBC_ReadU32BE(&RxData[9]);
    Meta.LastChunkSize  = RxData[13];
    Meta.ChunkCount     = PAYUEL_OBC_ReadU16BE(&RxData[14]);

    if (Meta.Status != 0U)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectHwStatus("DownloadMeta", Meta.Status);
    }

    if (Meta.CameraID != CameraID || Meta.ImageIndex != ImageNumber)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: DownloadMeta response mismatch req=(%u,%u) rsp=(%u,%u)",
                          CameraID, ImageNumber, Meta.CameraID, Meta.ImageIndex);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (Meta.ImageValid != 1U)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: DownloadMeta image invalid camera=%u image=%u valid=%u",
                          Meta.CameraID, Meta.ImageIndex, Meta.ImageValid);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (Meta.ChunkCount == 0U || Meta.LastChunkSize == 0U || Meta.LastChunkSize > PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: DownloadMeta invalid chunk info total=%u last=%u",
                          Meta.ChunkCount, Meta.LastChunkSize);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    PAYUEL_OBC_Data.ImageMetaSlot      = Meta.CameraID;
    PAYUEL_OBC_Data.ImageMetaNumber    = Meta.ImageIndex;
    PAYUEL_OBC_Data.ImageTotalChunks   = Meta.ChunkCount;
    PAYUEL_OBC_Data.ImageLastChunkSize = Meta.LastChunkSize;
    PAYUEL_OBC_Data.ImageFileCrc32     = Meta.ImageFileCRC32;

    if (MetaOut != NULL)
    {
        *MetaOut = Meta;
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_OBC_RequestSensorMeta(uint8_t DataSlot, uint8_t DataNumber,
                                          PAYUEL_OBC_SensorMetaInfo_t *MetaOut, uint8_t *ReturnTypeOut,
                                          uint8_t *PayloadErrorPacketOut)
{
    uint8_t                  TxData[5];
    uint8_t                  RxData[18] = {0};
    int32                    rx_len;
    CFE_Status_t             status = CFE_SUCCESS;
    PAYUEL_OBC_SensorMetaInfo_t Meta = {0};

    if (ReturnTypeOut != NULL)
    {
        /* Assume success first, then downgrade if any stage fails. */
        *ReturnTypeOut = RPT_RETTYPE_SUCCESS;
    }

    /* Request format: [cmd][data slot][data number][crc16]. */
    TxData[0] = PAYUEL_OBC_ID_SENSOR_META;
    TxData[1] = DataSlot;
    TxData[2] = DataNumber;
    PAYUEL_OBC_AppendCrc16(TxData, 3);

    /* Send the request and wait for the fixed-length sensor metadata response frame. */
    rx_len = CFE_SRL_ApiTransactionCSP(
        PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
        TxData, sizeof(TxData),
        RxData, sizeof(RxData));

    status = PAYUEL_OBC_ValidateResponse("SensorMeta", PAYUEL_OBC_ID_SENSOR_META,
                                         RxData, rx_len, sizeof(RxData),
                                         ReturnTypeOut, PayloadErrorPacketOut);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    Meta.Status        = RxData[1];
    Meta.DataSlot      = RxData[2];
    Meta.BinValid      = RxData[3];
    Meta.DataIndex     = RxData[4];
    Meta.BinSize       = PAYUEL_OBC_ReadU32BE(&RxData[5]);
    Meta.BinFileCRC32  = PAYUEL_OBC_ReadU32BE(&RxData[9]);
    Meta.LastChunkSize = RxData[13];
    Meta.ChunkCount    = PAYUEL_OBC_ReadU16BE(&RxData[14]);

    if (Meta.Status != 0U)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectHwStatus("SensorMeta", Meta.Status);
    }

    if (Meta.DataSlot != DataSlot || Meta.DataIndex != DataNumber)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: SensorMeta response mismatch req=(%u,%u) rsp=(%u,%u)",
                          DataSlot, DataNumber, Meta.DataSlot, Meta.DataIndex);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (Meta.BinValid != 1U)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: SensorMeta bin invalid slot=%u data=%u valid=%u",
                          Meta.DataSlot, Meta.DataIndex, Meta.BinValid);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (Meta.ChunkCount == 0U || Meta.LastChunkSize == 0U || Meta.LastChunkSize > PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: SensorMeta invalid chunk info total=%u last=%u",
                          Meta.ChunkCount, Meta.LastChunkSize);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    PAYUEL_OBC_Data.SensorMetaSlot      = Meta.DataSlot;
    PAYUEL_OBC_Data.SensorMetaNumber    = Meta.DataIndex;
    PAYUEL_OBC_Data.SensorTotalChunks   = Meta.ChunkCount;
    PAYUEL_OBC_Data.SensorLastChunkSize = Meta.LastChunkSize;
    PAYUEL_OBC_Data.SensorFileCrc32     = Meta.BinFileCRC32;

    if (MetaOut != NULL)
    {
        *MetaOut = Meta;
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_OBC_RequestImageChunk(uint8_t CameraID, uint8_t ImageNumber, uint16_t ChunkNumber,
                                          size_t ValidDataLength, uint8_t *ChunkDataOut, uint8_t *ReturnTypeOut,
                                          uint8_t *PayloadErrorPacketOut)
{
    uint8_t      TxData[7];
    uint8_t      RxData[PAYUEL_OBC_CHUNK_RESPONSE_SIZE] = {0};
    int32        rx_len;
    CFE_Status_t status = CFE_SUCCESS;
    uint8_t      rsp_status;
    uint16_t     rsp_image_num;
    uint16_t     rsp_chunk_num;
    size_t       expected_rx_len;

    if (ReturnTypeOut != NULL)
    {
        /* Default to success and only change this when we know the failure source. */
        *ReturnTypeOut = RPT_RETTYPE_SUCCESS;
    }

    if (ValidDataLength == 0U || ValidDataLength > PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        /* Callers must already know the expected valid byte count for this chunk. */
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_APP;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: ChunkDownload invalid data length %u",
                          (unsigned int)ValidDataLength);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_SB_BAD_ARGUMENT;
    }

    expected_rx_len = 7U + ValidDataLength + 2U;

    /* Request format: [cmd][camera][image][chunk number][crc16]. */
    TxData[0] = PAYUEL_OBC_ID_CHUNK_DOWNLOAD;
    TxData[1] = CameraID;
    TxData[2] = ImageNumber;
    PAYUEL_OBC_WriteU16BE(&TxData[3], ChunkNumber);
    PAYUEL_OBC_AppendCrc16(TxData, 5);

    /* Legacy SPI receive path kept commented for reference.
     * CFE_SRL_IO_Param_t spiParam = {0};
     * int32              spiStatus;
     *
     * spiParam.TxData   = TxData;
     * spiParam.TxSize   = sizeof(TxData);
     * spiParam.RxData   = RxData;
     * spiParam.RxSize   = sizeof(RxData);
     * spiParam.Interval = PAYUEL_OBC_SPI_INTERVAL_US;
     *
     * spiStatus = CFE_SRL_ApiRead(PAYUEL_OBC_Data.SpiHandle, &spiParam);
     */
    /* Receive variable-length chunk replies, including 4-byte error packets. */
    rx_len = CFE_SRL_ApiTransactionCSP(
        PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
        TxData, sizeof(TxData),
        RxData, -1);

    status = PAYUEL_OBC_ValidateChunkResponse("ChunkDownload", PAYUEL_OBC_ID_CHUNK_DOWNLOAD,
                                              RxData, rx_len, expected_rx_len,
                                              ReturnTypeOut, PayloadErrorPacketOut);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    rsp_status    = RxData[1];
    rsp_image_num = PAYUEL_OBC_ReadU16BE(&RxData[3]);
    rsp_chunk_num = PAYUEL_OBC_ReadU16BE(&RxData[5]);

    if (rsp_status != 0U)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectHwStatus("ChunkDownload", rsp_status);
    }

    if (RxData[2] != CameraID ||
        rsp_image_num != ImageNumber ||
        rsp_chunk_num != ChunkNumber)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: ChunkDownload header mismatch req=(%u,%u,%u) rsp=(%u,%u,%u)",
                          CameraID, ImageNumber, ChunkNumber, RxData[2], rsp_image_num, rsp_chunk_num);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (ChunkDataOut != NULL)
    {
        memcpy(ChunkDataOut, &RxData[7], ValidDataLength);
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_OBC_RequestSensorChunk(uint8_t DataSlot, uint8_t DataNumber, uint16_t ChunkNumber,
                                           size_t ValidDataLength, uint8_t *ChunkDataOut, uint8_t *ReturnTypeOut,
                                           uint8_t *PayloadErrorPacketOut)
{
    uint8_t      TxData[7];
    uint8_t      RxData[PAYUEL_OBC_CHUNK_RESPONSE_SIZE] = {0};
    int32        rx_len;
    CFE_Status_t status = CFE_SUCCESS;
    uint8_t      rsp_status;
    uint16_t     rsp_chunk_num;
    size_t       expected_rx_len;

    if (ReturnTypeOut != NULL)
    {
        /* Default to success and only change this when we know the failure source. */
        *ReturnTypeOut = RPT_RETTYPE_SUCCESS;
    }

    if (ValidDataLength == 0U || ValidDataLength > PAYUEL_OBC_CHUNK_DATA_SIZE)
    {
        /* Callers must already know the expected valid byte count for this chunk. */
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_APP;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: SensorChunk invalid data length %u",
                          (unsigned int)ValidDataLength);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_SB_BAD_ARGUMENT;
    }

    expected_rx_len = 6U + ValidDataLength + 2U;

    /* Request format: [cmd][slot][sensor number][chunk number][crc16]. */
    TxData[0] = PAYUEL_OBC_ID_SENSOR_CHUNK;
    TxData[1] = DataSlot;
    TxData[2] = DataNumber;
    PAYUEL_OBC_WriteU16BE(&TxData[3], ChunkNumber);
    PAYUEL_OBC_AppendCrc16(TxData, 5);

    /* Legacy SPI receive path kept commented for reference.
     * CFE_SRL_IO_Param_t spiParam = {0};
     * int32              spiStatus;
     *
     * spiParam.TxData   = TxData;
     * spiParam.TxSize   = sizeof(TxData);
     * spiParam.RxData   = RxData;
     * spiParam.RxSize   = sizeof(RxData);
     * spiParam.Interval = PAYUEL_OBC_SPI_INTERVAL_US;
     *
     * spiStatus = CFE_SRL_ApiRead(PAYUEL_OBC_Data.SpiHandle, &spiParam);
     */
    /* Receive variable-length chunk replies, including 4-byte error packets. */
    rx_len = CFE_SRL_ApiTransactionCSP(
        PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
        TxData, sizeof(TxData),
        RxData, -1);

    status = PAYUEL_OBC_ValidateChunkResponse("SensorChunk", PAYUEL_OBC_ID_SENSOR_CHUNK,
                                              RxData, rx_len, expected_rx_len,
                                              ReturnTypeOut, PayloadErrorPacketOut);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    rsp_status    = RxData[1];
    rsp_chunk_num = PAYUEL_OBC_ReadU16BE(&RxData[4]);

    if (rsp_status != 0U)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        return PAYUEL_OBC_RejectHwStatus("SensorChunk", rsp_status);
    }

    if (RxData[2] != DataSlot ||
        RxData[3] != DataNumber ||
        rsp_chunk_num != ChunkNumber)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_HW;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: SensorChunk header mismatch req=(%u,%u,%u) rsp=(%u,%u,%u)",
                          DataSlot, DataNumber, ChunkNumber, RxData[2], RxData[3], rsp_chunk_num);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (ChunkDataOut != NULL)
    {
        memcpy(ChunkDataOut, &RxData[6], ValidDataLength);
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_OBC_ComputeFileCrc32(const char *FileName, uint32_t *FileCrc32Out)
{
    uint8_t  Buffer[PAYUEL_OBC_FILE_CRC_BUFFER_SIZE];
    uint32_t Crc = 0xFFFFFFFFu;
    ssize_t  ReadSize;
    int      Fd;

    if (FileName == NULL || FileCrc32Out == NULL)
    {
        return CFE_SB_BAD_ARGUMENT;
    }

    /* Open the completed file and stream it through the incremental CRC32 helper. */
    Fd = open(FileName, O_RDONLY);
    if (Fd < 0)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Failed to open %s for CRC32", FileName);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    do
    {
        ReadSize = read(Fd, Buffer, sizeof(Buffer));
        if (ReadSize > 0)
        {
            /* Each read extends the running CRC instead of loading the whole file into memory. */
            Crc = PAYUEL_OBC_Crc32Update(Crc, Buffer, (size_t)ReadSize);
        }
    } while (ReadSize > 0);

    close(Fd);

    if (ReadSize < 0)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Failed to read %s during CRC32", FileName);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    *FileCrc32Out = Crc ^ 0xFFFFFFFFu;
    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_OBC_TblValidationFunc(void *TblData)
{
    CFE_Status_t              ReturnCode = CFE_SUCCESS;
    PAYUEL_OBC_ExampleTable_t *TblDataPtr = (PAYUEL_OBC_ExampleTable_t *)TblData;

    /* Keep the example table within the mission-configured range. */
    if (TblDataPtr->Int1 > PAYUEL_OBC_TBL_ELEMENT_1_MAX)
    {
        ReturnCode = PAYUEL_OBC_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

void PAYUEL_OBC_GetCrc(const char *TableName)
{
    CFE_Status_t   status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    /* Helper for debugging table contents from the system log. */
    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAYUEL_OBC: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("PAYUEL_OBC: CRC: 0x%08lX\n", (unsigned long)Crc);
    }
}
