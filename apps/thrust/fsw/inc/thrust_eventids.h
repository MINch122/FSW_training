#ifndef THRUST_EVENTIDS_H
#define THRUST_EVENTIDS_H

/**
 * @file
 * THRUST App Event IDs
 *
 * CFE_EVS_SendEvent() 의 EventID 파라미터로 사용.
 */

/* 앱 시작/초기화 */
#define THRUST_INIT_INF_EID         1  /* 앱 초기화 완료 */
#define THRUST_PIPE_ERR_EID         2  /* SB 파이프/구독 오류 */
#define THRUST_UART_INIT_ERR_EID    3  /* UART 초기화 실패 */

/* 메시지 라우팅 오류 */
#define THRUST_MID_ERR_EID          4  /* 알 수 없는 MsgID */
#define THRUST_CC_ERR_EID           5  /* 알 수 없는 CommandCode */
#define THRUST_LEN_ERR_EID          6  /* 메시지 길이 불일치 */

/* 명령 실행 오류 */
#define THRUST_ARM_INTERLOCK_EID    7  /* Arm 거부 - Interlock 활성 */
#define THRUST_FIRE_STATE_ERR_EID   8  /* Fire 거부 - ARMED 상태 아님 */
#define THRUST_FIRE_PARAM_ERR_EID   9  /* Fire 거부 - 파라미터 오류 */

/* 응답 상태 이벤트 */
#define THRUST_FAULT_EID           11  /* FaultFlags 감지 */
#define THRUST_INTERLOCK_EID       12  /* InterlockFlags 감지 */

/* 기본 명령 이벤트 */
#define THRUST_NOOP_INF_EID        13  /* NOOP 명령 수신 */
#define THRUST_RESET_INF_EID       14  /* ResetCounters 명령 수신 */

/* 공통 명령 실행 결과 */
#define THRUST_CMD_INF_EID         16  /* 명령 성공 */
#define THRUST_CMD_ERR_EID         17  /* 명령 실패 */

#endif /* THRUST_EVENTIDS_H */
