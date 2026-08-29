#ifndef MISSION_INTERNAL_CFG_H
#define MISSION_INTERNAL_CFG_H

#include "rpt_interface_cfg.h"

/* Pre definition to use debug */
#define MISSION_DEBUG
// #undef MISSION_DEBUG
/*----End of Pre definition----*/


/* Pipe Depth */
#define MISSION_PIPE_DEPTH      8

#define MISSION_LEOP_INITIAL_WAIT_SEC         (45u * 60u) // 45
#define MISSION_LEOP_BURN_DURATION_SEC        90u         
#define MISSION_LEOP_POST_BURN_WAIT_SEC       (30u * 60u) // 30
#define MISSION_LEOP_SAVE_INTERVAL_SEC        5u        
#define MISSION_LEOP_WAIT_LOG_SEC             30u
#define MISSION_LEOP_DATA_PATH                "/cf/leop.bin"
#define MISSION_LEOP_TEMP_DATA_PATH           "/cf/leop.tmp"
#define MISSION_LEOP_FILE_SIGNATURE           0x4C454F50u
#define MISSION_LEOP_FILE_VERSION             7u
#define MISSION_LEOP_TASK_NAME                "MISSION_LEOP"
#define MISSION_LEOP_TASK_STACK_SIZE          8192u
#define MISSION_LEOP_TASK_PRIORITY            7u
#define MISSION_LEOP_SEQUENCE_REPEAT_COUNT    10u

typedef enum {
    MISSION_LEOP_STATE_INITIAL_WAIT = 0,
    MISSION_LEOP_STATE_BURN,
    MISSION_LEOP_STATE_TO_ENABLE,
    MISSION_LEOP_STATE_POST_WAIT,
    MISSION_LEOP_STATE_COMPLETE
} MISSION_LEOP_State_t;

typedef struct {
    uint32 Signature;
    uint16 Version;
    uint8  LeopCycleCount;
    uint8  LeopState;
    uint32 LeopWaitElapsedSec;
    uint32 CRC;
} MISSION_LEOP_FileData_t;

#endif
