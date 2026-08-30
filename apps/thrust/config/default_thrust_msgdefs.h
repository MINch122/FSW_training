#ifndef THRUST_MSGDEFS_H
#define THRUST_MSGDEFS_H

#include "common_types.h"
#include "thrust_fcncodes.h"

/**
 * @file
 * THRUST App Message Payload Definitions
 *
 * 헤더(CommandHeader/TelemetryHeader) 없이 페이로드만 정의.
 * 완전한 SB 메시지 구조체는 thrust_msgstruct.h 에 있다.
 */

/* ==========================================================
 * Command Payloads (Ground → OBC)
 * ========================================================== */

/** THRUST_SET_MODE_CC — ICD MsgID=3 */
typedef struct {
    uint32_t TargetMode;  /* THRUST_MODE_* 중 하나 */
} __attribute__((packed)) THRUST_SetMode_Payload_t;

/** THRUST_RESET_MODULE_CC — ICD MsgID=2 */
typedef struct {
    uint32_t TargetMode;
} __attribute__((packed)) THRUST_ResetModule_Payload_t;

/** THRUST_ARM_CC — ICD MsgID=4 */
typedef struct {
    uint16_t ArmKey;      /* ICD: 0xA55A */
    uint16_t TimeoutSec;  /* seconds */
} __attribute__((packed)) THRUST_Arm_Payload_t;

/** THRUST_MAIN_FIRE_CC — ICD MsgID=20 */
typedef struct {
    uint32_t FireTime_ms;      /* 총 분사 시간 (ms) */
    uint16_t IgnitionTime_ms;  /* 점화 지속 시간 (ms), < FireTime_ms 강제 */
    uint16_t FireKey;          /* ICD: 0xA55A */
} __attribute__((packed)) THRUST_MainFire_Payload_t;

/** THRUST_CG_PULSE_CC — ICD MsgID=30 */
typedef struct {
    uint16_t ThrusterId;     /* CG 스러스터 ID */
    uint32_t PulseWidth_ms;  /* 펄스 폭 (ms) */
    uint16_t FireKey;        /* ICD: 0xA55A */
} __attribute__((packed)) THRUST_CGPulse_Payload_t;

/** THRUST_SCH_HK_ENABLE_CC — SCH-triggered HK enable switch */
typedef struct {
    uint8_t ScheduledHkEnabled;  /* 0: disable, 1: enable */
} __attribute__((packed)) THRUST_ScheduledHkEnable_Payload_t;

/* ==========================================================
 * Telemetry Payloads (OBC → Ground)
 * ========================================================== */

/** HK TLM Payload — ICD 4.3.7/4.3.8 데이터 + 앱 카운터 */
typedef struct {
    /* 압력 센서 4채널 (ICD 4.3.8) */
    float    Pressure_CH0;
    float    Pressure_CH1;
    float    Pressure_CH2;
    float    Pressure_CH3;
    /* 온도 센서 6채널 (ICD 4.3.8) */
    float    Temp_CH0;
    float    Temp_CH1;
    float    Temp_CH2;
    float    Temp_CH3;
    float    Temp_CH4;
    float    Temp_CH5;
    /* iG4U 내부 상태 (ICD 4.3.8) */
    uint32_t Status;
    /* 앱 카운터 */
    uint8_t  CmdCounter;
    uint8_t  ErrCounter;
} __attribute__((packed)) THRUST_HkTlm_Payload_t;

/** Status TLM Payload — ICD 4.3.9/4.3.10 데이터 */
typedef struct {
    uint32_t CurrentMode;      /* ICD 5.1 상태 머신 현재 모드 */
    uint16_t CurrentStatus;    /* iG4U Current Status */
    uint16_t FaultFlags;       /* Fault 비트필드 */
    uint16_t InterlockFlags;   /* Interlock 비트필드 */
    uint8_t  LastMsgId;        /* 마지막 처리 MsgID (진단) */
    uint8_t  LastResult;       /* 마지막 Result Code (진단) */
} __attribute__((packed)) THRUST_StatusTlm_Payload_t;

/** REQUEST HK의 RPT ReturnValue — 장치/CCSDS 헤더와 중복 timestamp 제외 */
typedef struct {
    float    Pressure_CH0;
    float    Pressure_CH1;
    float    Pressure_CH2;
    float    Pressure_CH3;
    float    Temp_CH0;
    float    Temp_CH1;
    float    Temp_CH2;
    float    Temp_CH3;
    float    Temp_CH4;
    float    Temp_CH5;
    uint32_t Status;
} __attribute__((packed)) THRUST_HkReport_Payload_t;

/** REQUEST STATUS의 RPT ReturnValue — ResultCode/Reserved 제외 */
typedef THRUST_StatusTlm_Payload_t THRUST_StatusReport_Payload_t;

/* ==========================================================
 * App-Level Error Codes
 * ========================================================== */
#define THRUST_ERR_INVALID_STATE   (-1)  /* 상태 머신 모드 불일치    */
#define THRUST_ERR_INVALID_PARAM   (-2)  /* 파라미터 유효성 실패     */
#define THRUST_ERR_INTERLOCK       (-3)  /* Interlock 조건 미충족   */

#endif /* THRUST_MSGDEFS_H */
