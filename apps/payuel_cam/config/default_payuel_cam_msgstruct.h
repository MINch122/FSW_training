#ifndef PAYUEL_CAM_MSGSTRUCT_H
#define PAYUEL_CAM_MSGSTRUCT_H

#include "payuel_cam_mission_cfg.h"
#include "payuel_cam_msgdefs.h"
#include "cfe_msg_hdr.h"
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------
 * Ground Command Structures
 * ----------------------------------------------------------------------- */

typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_CAM_NoopCmd_t;
typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_CAM_ResetCountersCmd_t;

/**
 * 0x41 - CAPTURE (RAW 생성)
 * ICD TX: [0x41][ImageSlot][CameraNumber][CRC16(BE)] = 5 bytes
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8  ImageSlot;    /**< ICD: slot */
    uint8  CameraNumber; /**< ICD: camNo */
} PAYUEL_CAM_ShotCmd_t;

/**
 * 0x42 - HEALTHCHECK
 * ICD TX: [0x42][CRC16(BE)] = 3 bytes
 */
typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_CAM_HealthCheckCmd_t;

/**
 * 0x43 - POSTPROCESS (사진 후처리)
 * ICD TX: [0x43][ImageSlot][ImageNumber][CRC16(BE)] = 5 bytes
 * The spreadsheet ICD uses ImageNumber here and is authoritative.
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 ImageSlot;   /**< ICD: slot */
    uint8 ImageNumber; /**< ICD: imageNo */
} PAYUEL_CAM_ProcessBinningCmd_t;

/**
 * 0x44 - DOWNLOAD META
 * ICD TX: [0x44][ImageSlot][ImageNumber][CRC16(BE)] = 5 bytes
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 ImageSlot;   /**< ICD: slot */
    uint8 ImageNumber; /**< ICD: imageNo */
} PAYUEL_CAM_DownloadMetaCmd_t;

/**
 * 0x45 - DOWNLOAD CHUNK
 * ICD TX: [0x45][ImageSlot][ImageNumber][ChunkNumber(uint16 BE)][CRC16(BE)] = 7 bytes
 * ICD RX: fixed 256 bytes, CRC32 over Cmd..valid data (padding excluded)
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8  ImageSlot;    /**< ICD: slot */
    uint8  ImageNumber;  /**< ICD: imageNo */
    uint16 ChunkNumber;  /**< ICD: chunkIdx */
} PAYUEL_CAM_ChunkDownloadCmd_t;

/**
 * cFS aggregate command
 * Runs 0x44 + all required 0x45 transfers in a child task and verifies FileCRC32.
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 ImageSlot;
    uint8 ImageNumber;
} PAYUEL_CAM_DownloadImageCmd_t;

/**
 * 0x46 - BEACON (상태 조회)
 */
typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_CAM_SendBcnCmd_t;

/* -----------------------------------------------------------------------
 * Telemetry Payload Structures
 * ----------------------------------------------------------------------- */

/**
 * 0x46 - CAM I/F Beacon telemetry payload
 */
typedef struct
{
    uint8_t  PI_Boot_State;
    uint16_t PI_Boot_Count;
    uint8_t  CAM_Detect_State;
    uint16_t CAM_Shutter_Count;
    int16_t  CPU_Temp_x100;
    uint32_t Free_Disk_MB;
} PAYUEL_CAM_Bcn_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAYUEL_CAM_Bcn_Payload_t  Payload;
} PAYUEL_CAM_BcnTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Payload;
} PAYUEL_CAM_RptTlm_t;

#endif /* PAYUEL_CAM_MSGSTRUCT_H */
