#ifndef MISSION_MSGDEFS_H
#define MISSION_MSGDEFS_H

#include "common_types.h"
#include "mission_fcncodes.h"


typedef struct MISSION_HkTlm_Payload{
    uint8_t  CmdCounter;
    uint8_t  CmdErrCounter;
    uint8_t  LeopWaitComplete;
    uint32_t LeopWaitElapsedSec;
    uint32_t LeopWaitRemainingSec;
    uint8_t  LeopUartDeployTryCount;
    uint8_t  LeopGpioBurnTryCount;
    uint8_t  LeopState;
    uint8_t  LeopUtrxRxBytesInitialized;
    uint8_t  LeopUtrxRxBytesIncreased;
    uint32_t LeopUtrxInitRxBytes;
    uint32_t LeopUtrxRxData;
} __attribute__((packed)) MISSION_HkTlm_Payload_t;

typedef MISSION_HkTlm_Payload_t MISSION_BcnTlm_Payload_t;

#endif
