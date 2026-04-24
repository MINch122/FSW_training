#ifndef PAYUEL_CAM_UTILS_H
#define PAYUEL_CAM_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cfe.h"
#include "payuel_cam_tbl.h"

typedef struct
{
    uint8  ImageSlot;
    uint8  ImageNumber;
    uint16 TotalChunks;
    uint8  LastChunkSize;
    uint32 FileCrc32;
} PAYUEL_CAM_ImageMetaInfo_t;

uint16_t     PAYUEL_CAM_ReadU16BE(const uint8_t *Data);
int16_t      PAYUEL_CAM_ReadI16BE(const uint8_t *Data);
uint32_t     PAYUEL_CAM_ReadU32BE(const uint8_t *Data);
void         PAYUEL_CAM_WriteU16BE(uint8_t *Data, uint16_t Value);
void         PAYUEL_CAM_WriteU32BE(uint8_t *Data, uint32_t Value);
bool         PAYUEL_CAM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void         PAYUEL_CAM_AppendCrc16(uint8_t *Buf, size_t PayloadLength);
bool         PAYUEL_CAM_VerifyCrc16(const uint8_t *Data, size_t TotalLength);
bool         PAYUEL_CAM_VerifyChunkCrc32(const uint8_t *Data, size_t ValidDataLength);
void         PAYUEL_CAM_ClearImageMetaCache(void);
void         PAYUEL_CAM_ReportCmdStatus(CFE_SB_MsgId_t MsgId, uint16 CommandCode, int32 Status,
                                        const uint8_t *Data, uint16 DataLength, uint8 ReturnType);
int32        PAYUEL_CAM_DownloadBusyError(const char *CmdName);
int32        PAYUEL_CAM_LockHardware(const char *Context);
void         PAYUEL_CAM_UnlockHardware(const char *Context);
int32        PAYUEL_CAM_RspLenError(const char *CmdName, int32 ActualLength, size_t ExpectedLength);
int32        PAYUEL_CAM_HwStatusError(const char *CmdName, uint8_t HwStatus);
CFE_Status_t PAYUEL_CAM_ValidateResponse(const char *CmdName, uint8_t ExpectedCmd, const uint8_t *RxData,
                                         int32 RspLen, size_t ExpectedLength, uint8_t *ReturnTypeOut,
                                         uint8_t *PayloadErrorPacketOut);
size_t       PAYUEL_CAM_GetChunkDataLenFromMeta(const PAYUEL_CAM_ImageMetaInfo_t *Meta, uint16_t ChunkNumber);
bool         PAYUEL_CAM_GetCachedImageMeta(uint8_t ImageSlot, uint8_t ImageNumber,
                                           PAYUEL_CAM_ImageMetaInfo_t *MetaOut);
CFE_Status_t PAYUEL_CAM_RequestImageMeta(uint8_t ImageSlot, uint8_t ImageNumber,
                                         PAYUEL_CAM_ImageMetaInfo_t *MetaOut, uint8_t *ReturnTypeOut,
                                         uint8_t *PayloadErrorPacketOut);
CFE_Status_t PAYUEL_CAM_RequestImageChunk(uint8_t ImageSlot, uint8_t ImageNumber, uint16_t ChunkNumber,
                                          size_t ValidDataLength, uint8_t *ChunkDataOut, uint8_t *ReturnTypeOut,
                                          uint8_t *PayloadErrorPacketOut);
CFE_Status_t PAYUEL_CAM_ComputeFileCrc32(const char *FileName, uint32_t *FileCrc32Out);
CFE_Status_t PAYUEL_CAM_TblValidationFunc(void *TblData);
void         PAYUEL_CAM_GetCrc(const char *TableName);

#endif /* PAYUEL_CAM_UTILS_H */
