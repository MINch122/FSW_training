#ifndef PAYUEL_OBC_UTILS_H
#define PAYUEL_OBC_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cfe.h"
#include "payuel_obc_tbl.h"

typedef struct
{
    uint8  CameraID;
    uint8  ImageIndex;
    uint16 ChunkCount;
    uint8  LastChunkSize;
    uint32 ImageFileCRC32;
} PAYUEL_OBC_ImageMetaInfo_t;

typedef struct
{
    uint8  DataSlot;
    uint8  DataIndex;
    uint16 ChunkCount;
    uint8  LastChunkSize;
    uint32 BinFileCRC32;
} PAYUEL_OBC_SensorMetaInfo_t;

uint16_t     PAYUEL_OBC_ReadU16BE(const uint8_t *Data);
uint32_t     PAYUEL_OBC_ReadU32BE(const uint8_t *Data);
void         PAYUEL_OBC_WriteU16BE(uint8_t *Data, uint16_t Value);
void         PAYUEL_OBC_WriteU32BE(uint8_t *Data, uint32_t Value);
bool         PAYUEL_OBC_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void         PAYUEL_OBC_AppendCrc16(uint8_t *Buf, size_t PayloadLength);
bool         PAYUEL_OBC_VerifyCrc16(const uint8_t *Data, size_t TotalLength);
void         PAYUEL_OBC_ClearImageMetaCache(void);
void         PAYUEL_OBC_ClearSensorMetaCache(void);
void         PAYUEL_OBC_ReportCmdStatus(CFE_SB_MsgId_t MsgId, uint16 CommandCode, int32 Status,
                                        const uint8_t *Data, uint16 DataLength, uint8 ReturnType);
int32        PAYUEL_OBC_LockHardware(const char *Context);
void         PAYUEL_OBC_UnlockHardware(const char *Context);
CFE_Status_t PAYUEL_OBC_RejectHwStatus(const char *CmdName, uint8_t HwStatus);
CFE_Status_t PAYUEL_OBC_ValidateResponse(const char *CmdName, uint8_t ExpectedCmd, const uint8_t *RxData,
                                         int32 RspLen, size_t ExpectedLength, uint8_t *ReturnTypeOut,
                                         uint8_t *PayloadErrorPacketOut);
size_t       PAYUEL_OBC_GetChunkDataLenFromImageMeta(const PAYUEL_OBC_ImageMetaInfo_t *Meta, uint16_t ChunkNumber);
size_t       PAYUEL_OBC_GetChunkDataLenFromSensorMeta(const PAYUEL_OBC_SensorMetaInfo_t *Meta, uint16_t ChunkNumber);
bool         PAYUEL_OBC_GetCachedImageMeta(uint8_t CameraID, uint8_t ImageNumber,
                                           PAYUEL_OBC_ImageMetaInfo_t *MetaOut);
bool         PAYUEL_OBC_GetCachedSensorMeta(uint8_t DataSlot, uint8_t DataNumber,
                                            PAYUEL_OBC_SensorMetaInfo_t *MetaOut);
CFE_Status_t PAYUEL_OBC_RequestImageMeta(uint8_t CameraID, uint8_t ImageNumber,
                                         PAYUEL_OBC_ImageMetaInfo_t *MetaOut, uint8_t *ReturnTypeOut,
                                         uint8_t *PayloadErrorPacketOut);
CFE_Status_t PAYUEL_OBC_RequestSensorMeta(uint8_t DataSlot, uint8_t DataNumber,
                                          PAYUEL_OBC_SensorMetaInfo_t *MetaOut, uint8_t *ReturnTypeOut,
                                          uint8_t *PayloadErrorPacketOut);
CFE_Status_t PAYUEL_OBC_RequestImageChunk(uint8_t CameraID, uint8_t ImageNumber, uint16_t ChunkNumber,
                                          size_t ValidDataLength, uint8_t *ChunkDataOut, uint8_t *ReturnTypeOut,
                                          uint8_t *PayloadErrorPacketOut);
CFE_Status_t PAYUEL_OBC_RequestSensorChunk(uint8_t DataSlot, uint8_t DataNumber, uint16_t ChunkNumber,
                                           size_t ValidDataLength, uint8_t *ChunkDataOut, uint8_t *ReturnTypeOut,
                                           uint8_t *PayloadErrorPacketOut);
CFE_Status_t PAYUEL_OBC_ComputeFileCrc32(const char *FileName, uint32_t *FileCrc32Out);
CFE_Status_t PAYUEL_OBC_TblValidationFunc(void *TblData);
void         PAYUEL_OBC_GetCrc(const char *TableName);

#endif /* PAYUEL_OBC_UTILS_H */
