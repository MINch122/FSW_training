#ifndef THRUST_PACKET_DEFS_H
#define THRUST_PACKET_DEFS_H

/**
 * @file thrust_packet_defs.h
 * @brief iG4U 패킷 구조체 정의 (ICD D02 기준)
 *
 * [패킷 구조 - ICD Section 4.1 표 1]
 *   Byte [0-1]    : Header   0xAA55
 *   Byte [2]      : Version  0x10 (Major=1, Minor=0)
 *   Byte [3]      : MsgType  0=TC, 1=TM, 2=Event
 *   Byte [4]      : MsgID    메시지 고유 ID
 *   Byte [5]      : Length   Payload 길이 (UINT8, 1바이트)
 *   Byte [6..N+5] : Payload  N바이트 데이터
 *   Byte [N+6..N+7] : CRC16
 *
 * [CRC 계산 범위 - ICD Section 4.3]
 *   Header 제외, Version부터 Payload 마지막 바이트까지
 *   즉: &pkt.Version ~ &pkt.Payload[Length-1]
 *
 * [엔디안 - ICD Section 4.3]
 *   모든 멀티바이트: LSB First (Little Endian)
 *
 * [ICD 내부 표기 주의]
 *   일부 Length 값은 같은 표의 필드/오프셋 합계와 다르다.
 *   이 파일의 payload 크기는 필드 정의와 바이너리 오프셋으로 계산한다.
 */

#include "common_types.h"

/* ===================================================================
 * 고정 상수
 * =================================================================== */
#define THRUST_PKT_HEADER       0xAA55
#define THRUST_PKT_VERSION      0x10
#define THRUST_PKT_MSGTYPE_TC   0
#define THRUST_PKT_MSGTYPE_TM   1
#define THRUST_PKT_MSGTYPE_EVENT 2

#define THRUST_RESULT_ACK       0
#define THRUST_RESULT_NACK      1

#define THRUST_ARM_KEY          0xA55A  /* Arm 명령 보안 키 */
#define THRUST_FIRE_KEY         0xA55A  /* Fire 명령 보안 키 */

/* ===================================================================
 * 메시지별 Payload 크기 (ICD Section 4.3 바이너리 오프셋 기준)
 * =================================================================== */
#define THRUST_N_DEFAULT_RESP   8   /* Result+Status+Fault+Interlock     */
#define THRUST_N_PING           0   /* Payload 없음                      */
#define THRUST_N_RESET_MODULE   4   /* Target Mode (UINT32)              */
#define THRUST_N_SET_MODE       4   /* Target Mode (UINT32)              */
#define THRUST_N_ARM            4   /* Arm Key + Timeout                 */
#define THRUST_N_DISARM         0   /* Payload 없음                      */
#define THRUST_N_REQUEST_HK     0   /* Payload 없음                      */
#define THRUST_N_REPLY_HK       48  /* Timestamp+Pressure×4+Temp×6+Status*/
#define THRUST_N_REQUEST_STATUS 0   /* Payload 없음                      */
#define THRUST_N_REPLY_STATUS   16  /* Mode+LastMsg+Result+Status+Fault+Interlock+Reserved */
#define THRUST_N_MAIN_FIRE      8   /* FireTime+IgnitionTime+FireKey      */
#define THRUST_N_MAIN_ABORT     0   /* Payload 없음                      */
#define THRUST_N_CG_PULSE       8   /* ThrusterID+PulseWidth+FireKey      */
#define THRUST_N_CG_ABORT       0   /* Payload 없음                      */
#define THRUST_N_REQUEST_FAULT  0   /* Payload 없음                      */
#define THRUST_N_REPLY_FAULT    40  /* Reserved [TBD]                    */
#define THRUST_N_CLEAR_FAULT    0   /* Payload 없음                      */

/* 최대 버퍼 크기 */
#define THRUST_N_MAX            200



/* ===================================================================
 * Payload 구조체 (ICD Section 4.3 기준)
 * app 팀은 이 파일을 include하여 사용할 것
 * =================================================================== */

// 아래의 msd id 말고는 payload length가 0이라 payload 구조체를 정의하지 않음

/* Default Response (Length=8) */
typedef struct {
    uint16_t ResultCode;        /* ACK=0 / NACK=1                      */
    uint16_t CurrentStatus;     /* 현재 상태                           */
    uint16_t FaultFlags;        /* Fault 플래그                        */
    uint16_t InterlockFlags;    /* Interlock 플래그                    */
} __attribute__((packed)) THRUST_DefaultResponse_t; //pading을 없애는 용도 모든 구조체에 적용.

/* Reply HK Data TM (MsgID=10, Length=48) */
typedef struct {
    uint32_t Timestamp_ms;      /* ms                                  */
    float    Pressure_CH0;      /* bar (FLOAT32)                       */
    float    Pressure_CH1;
    float    Pressure_CH2;
    float    Pressure_CH3;
    float    Temp_CH0;          /* Degree (FLOAT32)                    */
    float    Temp_CH1;
    float    Temp_CH2;
    float    Temp_CH3;
    float    Temp_CH4;
    float    Temp_CH5;
    uint32_t Status;            /* 상태 Bitfield                       */
} __attribute__((packed)) THRUST_HKData_Payload_t;

/* Reply Status TM (MsgID=11, Length=16) */
typedef struct {
    uint32_t CurrentMode;
    uint8_t  LastMsgId;
    uint8_t  LastResult;
    uint16_t ResultCode;
    uint16_t CurrentStatus;
    uint16_t FaultFlags;
    uint16_t InterlockFlags;
    uint16_t Reserved;
} __attribute__((packed)) THRUST_StatusData_Payload_t;

/* Reply Fault Log TM (MsgID=40, Length=40, TBD) */
typedef struct {
    uint8_t Reserved[40];       /* [TBD]                               */
} __attribute__((packed)) THRUST_FaultLogData_Payload_t;

/* ===================================================================
 * Little Endian 직렬화 매크로 (ICD Section 4.3 - LSB First)
 * =================================================================== */
#define PACK_U16_LE(buf, val) \
    do { \
        (buf)[0] = (uint8_t)( (val)       & 0xFF); \
        (buf)[1] = (uint8_t)(((val) >> 8) & 0xFF); \
    } while(0)

#define PACK_U32_LE(buf, val) \
    do { \
        (buf)[0] = (uint8_t)( (val)        & 0xFF); \
        (buf)[1] = (uint8_t)(((val) >>  8) & 0xFF); \
        (buf)[2] = (uint8_t)(((val) >> 16) & 0xFF); \
        (buf)[3] = (uint8_t)(((val) >> 24) & 0xFF); \
    } while(0)

#endif /* THRUST_PACKET_DEFS_H */
