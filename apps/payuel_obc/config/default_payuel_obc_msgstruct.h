#ifndef PAYUEL_OBC_MSGSTRUCT_H
#define PAYUEL_OBC_MSGSTRUCT_H

#include "payuel_obc_mission_cfg.h"
#include "payuel_obc_msgdefs.h"
#include "cfe_msg_hdr.h"
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------
 * Ground Command Structures
 * ----------------------------------------------------------------------- */

typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_OBC_NoopCmd_t;
typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_OBC_ResetCountersCmd_t;

/**
 * 0x20 - UEL OBC 비콘 요청 (파라미터 없음)
 */
typedef struct { CFE_MSG_CommandHeader_t CommandHeader; } PAYUEL_OBC_SendObcBcnCmd_t;

/**
 * 0x21 - 모터 모드 제어
 * TX: [0x21][L_Speed(int8)][R_Speed(int8)][Duration(uint8)][CRC16(BE)] = 6 bytes
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    int8   L_Speed;   /**< 좌측 모터 속도, -128~127 */
    int8   R_Speed;   /**< 우측 모터 속도, -128~127 */
    uint8  Duration;  /**< 동작 시간, 0~255 */
} PAYUEL_OBC_MotorModeCmd_t;

/**
 * 0x41 - 사진 촬영 요청
 * TX: [0x41][CameraID][ImageNumber][CRC16(BE)] = 5 bytes
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8  CameraID;  /**< 촬영에 사용할 카메라 ID, 0x61 또는 0x62 */
    uint8  ImageNumber; /**< 저장할 이미지 번호, 0~255 */
} PAYUEL_OBC_CamShotCmd_t;

/**
 * 0x44 - 이미지 다운로드 메타 요청
 * TX: [0x44][CameraID][ImageNumber][CRC16(BE)] = 5 bytes
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 CameraID;    /**< 다운로드할 카메라 ID, 0x61 또는 0x62 */
    uint8 ImageNumber; /**< 이미지 번호, 0~255 */
} PAYUEL_OBC_DownloadMetaCmd_t;

/**
 * 0x45 - 이미지 청크 다운로드 요청
 * TX: [0x45][CameraID][ImageNumber][ChunkNumber(uint16 BE)][CRC16(BE)] = 7 bytes
 * RX: [0x45][Status][CameraID][ImageNumber(uint16 BE)][ChunkNumber(uint16 BE)][Data][CRC16(BE)]
 *     = 7 + valid data length + 2 bytes (max 256)
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8  CameraID;     /**< 다운로드할 카메라 ID, 0x61 또는 0x62 */
    uint8  ImageNumber;  /**< 다운로드할 이미지 번호, 0~255 */
    uint16 ChunkNumber;  /**< 요청할 chunk 번호, 0 ~ (Total Chunk - 1) */
} PAYUEL_OBC_ChunkDownloadCmd_t;

/**
 * 0x54 - 센서 데이터 다운로드 메타 요청
 * TX: [0x54][DataSlot][DataNumber][CRC16(BE)] = 5 bytes
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 DataSlot;   /**< 다운로드할 데이터 슬롯 번호, 0~255 */
    uint8 DataNumber; /**< 슬롯 내 데이터 번호, 0~255 */
} PAYUEL_OBC_SensorMetaCmd_t;

/**
 * 0x55 - 센서 데이터 청크 다운로드 요청
 * TX: [0x55][DataSlot][DataNumber][ChunkNumber(uint16 BE)][CRC16(BE)] = 7 bytes
 * RX: [0x55][Status][DataSlot][DataNumber][ChunkNumber(uint16 BE)][Data][CRC16(BE)]
 *     = 6 + valid data length + 2 bytes (max 255)
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8  DataSlot;    /**< 다운로드할 데이터 슬롯 번호, 0~255 */
    uint8  DataNumber;  /**< 슬롯 내 데이터 번호, 0~255 */
    uint16 ChunkNumber; /**< 요청할 chunk 번호, 0 ~ (Total Chunk - 1) */
} PAYUEL_OBC_SensorChunkCmd_t;

/**
 * cFS aggregate image download command
 * Runs 0x44 + all required 0x45 transfers in a child task and verifies Image File CRC32.
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 CameraID;
    uint8 ImageNumber;
} PAYUEL_OBC_DownloadImageCmd_t;

/**
 * cFS aggregate sensor download command
 * Runs 0x54 + all required 0x55 transfers in a child task.
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 DataSlot;
    uint8 DataNumber;
} PAYUEL_OBC_DownloadSensorCmd_t;

/* -----------------------------------------------------------------------
 * Telemetry Payload Structures
 * ----------------------------------------------------------------------- */

/**
 * 0x20 - UEL OBC 비콘 텔레메트리 payload (UEL OBC ICD)
 * RX: [init_error(1)][boot_count(2)][boot_time(4)][shutter_count(2)]
 *     [alpha_temp(1)][beta_temp(1)][imu_temp(1)][tc1(1)][sd_mount_state(1)]
 *     = 14 bytes (excl. CRC)
 */
typedef struct
{
    uint8_t  InitError;       /**< 초기화 에러 코드, 0=정상 */
    uint16_t BootCount_a;       /**< 부팅 횟수, 0~65535 */
    uint32_t BootTime_a;        /**< 부팅 시간 (초), 0~4294967295 */
    uint16_t BootCount_b;
    uint32_t BootTime_b;
    uint16_t ShutterCount_a;    /**< 카메라 촬영 횟수, 0~65535 */
    uint16_t ShutterCount_b;
    int8_t   AlphaTemp;       /**< Alpha 온도, -128~127 */
    int8_t   BetaTemp;        /**< Beta 온도, -128~127 */
    int8_t   ImuTemp;         /**< IMU 온도, -128~127 */
    int8_t   TC1;             /**< 열전대 1 온도, -128~127 */
    uint8_t  SdMountState;    /**< SD 마운트 상태 (0: unmounted, 1: mounted) */
} PAYUEL_OBC_ObcBcn_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAYUEL_OBC_ObcBcn_Payload_t Payload;
} PAYUEL_OBC_ObcBcnTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Payload;
} PAYUEL_OBC_RptTlm_t;

#endif /* PAYUEL_OBC_MSGSTRUCT_H */
