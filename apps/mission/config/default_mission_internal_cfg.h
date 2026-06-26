#ifndef MISSION_INTERNAL_CFG_H
#define MISSION_INTERNAL_CFG_H

#include "rpt_interface_cfg.h"

/* Pre definition to use debug */
#define MISSION_DEBUG
// #undef MISSION_DEBUG
/*----End of Pre definition----*/


/* Pipe Depth */
#define MISSION_PIPE_DEPTH      8

#define MISSION_LEOP_WAIT_DURATION_SEC        (45u * 60u)
#define MISSION_LEOP_SAVE_INTERVAL_SEC        10u
#define MISSION_LEOP_RETRY_DELAY_SEC          30u
#define MISSION_LEOP_POST_BURN_WAIT_SEC       (1u * 40u)
#define MISSION_LEOP_POST_BURN_LOG_SEC        30u
#define MISSION_LEOP_DATA_PATH                "/cf/leop.bin"
#define MISSION_LEOP_FILE_SIGNATURE           0x4C454F50u
#define MISSION_LEOP_FILE_VERSION             2u
#define MISSION_LEOP_TASK_NAME                "MISSION_LEOP"
#define MISSION_LEOP_TASK_STACK_SIZE          8192u
#define MISSION_LEOP_TASK_PRIORITY            7u
#define MISSION_LEOP_MAX_GPIO_BURN_TRY_COUNT  3

typedef enum {
    MISSION_LEOP_STATE_WAITING = 0,
    MISSION_LEOP_STATE_WAIT_COMPLETE,
    MISSION_LEOP_STATE_TO_ENABLED,
    MISSION_LEOP_STATE_GPIO_HIGH_DONE,
    MISSION_LEOP_STATE_COMPLETE
} MISSION_LEOP_State_t;

typedef struct {
    uint32 Signature;
    uint16 Version;
    uint8  LeopWaitComplete;
    uint32 LeopWaitElapsedSec;
    uint32 LeopWaitRemainingSec;
    uint8  LeopUartDeployTryCount;
    uint8  LeopGpioBurnTryCount;
    uint8  LeopState;
    uint8  LeopToEnabled;
    uint8  LeopGpioDeployIssued;
    uint8  LeopUtrxRxBytesInitialized;
    uint8  Reserved[2];
    uint32 LeopUtrxInitRxBytes;
    uint32 CRC;
} MISSION_LEOP_FileData_t;

#endif
