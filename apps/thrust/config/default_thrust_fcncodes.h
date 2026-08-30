#ifndef THRUST_FCNCODES_H
#define THRUST_FCNCODES_H

/**
 * @file
 * THRUST App Command Function Codes (CC)
 *
 * 이 CC는 지상국이 OBC에 보내는 명령 번호다.
 * iG4U ICD의 MsgID(하드웨어 프로토콜)와는 별개다.
 * 매핑: CC → dispatch → cmds → utils(MsgID)
 */

/* cFS 표준 명령 */
#define THRUST_NOOP_CC            0
#define THRUST_RESET_COUNTERS_CC  1

/* iG4U 제어 명령 (ICD 표 2 기반) */
#define THRUST_PING_CC            2   /* ICD MsgID=1  */
#define THRUST_RESET_MODULE_CC    3   /* ICD MsgID=2  */
#define THRUST_SET_MODE_CC        4   /* ICD MsgID=3  */
#define THRUST_ARM_CC             5   /* ICD MsgID=4  */
#define THRUST_DISARM_CC          6   /* ICD MsgID=5  */
#define THRUST_REQ_STATUS_CC      7   /* ICD MsgID=11 */
#define THRUST_MAIN_FIRE_CC       8   /* ICD MsgID=20 */
#define THRUST_MAIN_ABORT_CC      9   /* ICD MsgID=21 */
#define THRUST_CG_PULSE_CC        10  /* ICD MsgID=30 */
#define THRUST_CG_ABORT_CC        11  /* ICD MsgID=31 */
#define THRUST_REQ_FAULT_LOG_CC   12  /* ICD MsgID=40 */
#define THRUST_CLEAR_FAULT_CC     13  /* ICD MsgID=41 */
#define THRUST_REQ_HK_CC          14  /* ICD MsgID=10 */
#define THRUST_SCH_HK_ENABLE_CC   15  /* Enable/disable SCH-triggered HK */

/* SCH 트리거 명령 (CC 없음 — HandleReport용 식별자) */
#define THRUST_SEND_HK_CC         THRUST_REQ_HK_CC

#endif /* THRUST_FCNCODES_H */
