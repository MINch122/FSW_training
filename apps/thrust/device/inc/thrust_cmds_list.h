#ifndef THRUST_CMD_LIST_H
#define THRUST_CMD_LIST_H

/**
 * @file thrust_cmds_list.h
 * @brief iG4U Device 레이어 함수 선언 (ICD D02 기준)
 *
 * 역할:
 *   - 패킷 구성 (Header ~ CRC)
 *   - RS-422 송신
 *   - 1바이트씩 타임아웃 기반 수신 및 헤더 동기화
 *   - CRC 검증
 *   - DefaultResponse를 파싱하여 outResp로 caller에게 전달
 *   - AppData 갱신 및 EVS 이벤트는 App 팀(PostCmdProcess) 담당
 *   - HK/Status/FaultLog Payload는 returnVal로 전달 (해석은 cFS 담당)
 */

#include "common_types.h"
#include "cfe.h"
#include "thrust_packet_defs.h"   /* Payload 구조체 */
#include "thrust_hal.h"           /* UART HAL */

/* ===================================================================
 * 수신 타임아웃 (바이트 단위)
 * 1바이트를 기다리는 최대 시간.
 * TBD: 장치 응답 시간 측정 후 확정 필요.
 * =================================================================== */
#define THRUST_RECV_TIMEOUT1_MS    100
#define THRUST_RECV_TIMEOUT2_MS    500

/* ===================================================================
 * Abort 명령 후 버퍼 플러시 대기 시간
 * TBD: 장치 ACK 응답 시간 측정 후 확정 필요.
 * =================================================================== */
#define THRUST_ABORT_FLUSH_MS     20

/* ===================================================================
 * Device 레이어 에러 코드
 * App 레이어 에러 코드(INVALID_STATE=-1, INVALID_PARAM=-2)와
 * 충돌하지 않도록 -3부터 할당
 * =================================================================== */
#define THRUST_ERR_WRITE          (-4)   /* UART 송신 실패             */
#define THRUST_ERR_TIMEOUT        (-5)   /* 수신 타임아웃              */
#define THRUST_ERR_CRC            (-6)   /* CRC 검증 실패              */
#define THRUST_ERR_NACK           (-7)   /* iG4U NACK 응답             */
#define THRUST_ERR_HEADER         (-8)   /* iG4U Header 오류           */
#define THRUST_ERR_PACKET         (-9)   /* MsgType/MsgID/Length 오류  */

/*DEVICE transport functions registration*/

typedef int32_t (*THRUST_TransportFn_t)(const void *tx_buf,
                                      uint16_t    tx_len,
                                      void       *rx_buf,
                                      int32_t     rx_size,
                                      uint16_t    timeout_ms);


// Register platform transport before calling any THRUST API
void THRUST_RegisterTransport(THRUST_TransportFn_t fn);

/* Return the raw response bytes received during the latest device transaction. */
void THRUST_GetLastResponse(const uint8_t **data, uint16_t *size);


/* ===================================================================
 * Group 1: 기본 제어
 * DefaultResponse를 파싱하여 outResp로 전달.
 * AppData 갱신은 App 팀(PostCmdProcess) 담당.
 * =================================================================== */
int32 THRUST_Ping(THRUST_DefaultResponse_t *outResp);
int32 THRUST_ResetModule(uint32_t targetMode, THRUST_DefaultResponse_t *outResp);
int32 THRUST_SetMode(uint32_t targetMode, THRUST_DefaultResponse_t *outResp);
int32 THRUST_ArmPropulsion(uint16_t armKey, uint16_t timeoutSec,
                            THRUST_DefaultResponse_t *outResp);
int32 THRUST_DisarmPropulsion(THRUST_DefaultResponse_t *outResp);

/* ===================================================================
 * Group 2: HK / 상태 요청
 * Payload 전체를 returnVal로 전달. 해석은 App 팀 담당.
 * =================================================================== */
int32 THRUST_GetHKData(THRUST_HKData_Payload_t *returnVal);
int32 THRUST_GetStatus(THRUST_StatusData_Payload_t *returnVal);

/* ===================================================================
 * Group 3: 주추력기
 * =================================================================== */
int32 THRUST_MainThrusterFire(uint32_t fireTime_ms, uint16_t ignitionTime_ms,
                               uint16_t fireKey,
                               THRUST_DefaultResponse_t *outResp);
int32 THRUST_MainThrusterAbort(THRUST_DefaultResponse_t *outResp);

/* ===================================================================
 * Group 4: 냉가스 추력기
 * =================================================================== */
int32 THRUST_CGThrusterPulse(uint16_t thrusterId, uint32_t pulseWidth_ms,
                              uint16_t fireKey,
                              THRUST_DefaultResponse_t *outResp);
int32 THRUST_CGThrusterAbort(THRUST_DefaultResponse_t *outResp);

/* ===================================================================
 * Group 5: Fault 관리
 * =================================================================== */
int32 THRUST_GetFaultLog(THRUST_FaultLogData_Payload_t *returnVal);
int32 THRUST_ClearFault(THRUST_DefaultResponse_t *outResp);

#endif /* THRUST_CMDS_LIST_H */
