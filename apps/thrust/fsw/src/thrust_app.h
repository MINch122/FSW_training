
#ifndef THRUST_APP_H
#define THRUST_APP_H

#include "cfe.h"                   // CFE_Status_t, CFE_SB_PipeId_t 등 cFE 기본 타입
#include "thrust_mission_cfg.h"    // 미션 레벨 설정
#include "thrust_platform_cfg.h"   // 플랫폼 레벨 설정 (PIPE_DEPTH 등)
#include "thrust_perfids.h"        // THRUST_PERF_ID
#include "thrust_msgids.h"         // THRUST_CMD_MID, THRUST_HK_TLM_MID 등
#include "thrust_msg.h"            // THRUST_HkTlm_t, THRUST_StatusTlm_t 등

/* 상태 머신 모드 (ICD 5.1절) */
#define THRUST_MODE_BOOT      0
#define THRUST_MODE_STANDBY   1
#define THRUST_MODE_ARMED     2
#define THRUST_MODE_FIRING    3
#define THRUST_MODE_SAFE      4
#define THRUST_MODE_FAULT     5

typedef struct {
    /* 명령 카운터 */
    uint8  CmdCounter;
    uint8  ErrCounter;

    /* 텔레메트리 패킷 */
    THRUST_HkTlm_t     HkTlm;      /* HK: 압력/온도/상태 */
    THRUST_StatusTlm_t StatusTlm;  /* Status: 모드/Fault/Interlock */

    /* 메인 루프 */
    uint32 RunStatus;

    /* SB 파이프 */
    CFE_SB_PipeId_t CommandPipe;
    char            PipeName[CFE_MISSION_MAX_API_LEN];
    uint16          PipeDepth;

    /* UART (RS-422) */
    int     UartFd;
    uint8_t ProtocolVersion;  /* ICD Version 필드 */

    /* iG4U 동기화 상태 — 매 응답마다 갱신 */
    uint32_t CurrentMode;       /* ICD Reply Status: Current Mode */
    uint16_t CurrentStatus;     /* ICD Default Response: Current Status */
    uint16_t FaultFlags;        /* ICD Default Response: Fault Flags */
    uint16_t InterlockFlags;    /* ICD Default Response: Interlock Flags */

    /* 진단 */
    uint8_t  LastMsgId;         /* ICD Reply Status: Last Msg ID */
    uint8_t  LastResult;        /* ICD Reply Status: Last Result */

} THRUST_AppData_t;

extern THRUST_AppData_t THRUST_AppData;

void         THRUST_AppMain(void);
CFE_Status_t THRUST_AppInit(void);

#endif /* THRUST_APP_H */
