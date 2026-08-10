#ifndef MISSION_MSGDEFS_H
#define MISSION_MSGDEFS_H

#include "common_types.h"

typedef struct MISSION_HkTlm_Payload{
    uint8_t  CmdErrCounter;
    uint8_t  LeopWaitComplete;
    uint32_t LeopWaitElapsedSec;
    uint32_t LeopWaitRemainingSec;
    uint8_t  LeopCycleCount;
    uint8_t  LeopState;
} __attribute__((packed)) MISSION_HkTlm_Payload_t;

#endif
