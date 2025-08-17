#ifndef PAYUZUT_MSGDEFS_H
#define PAYUZUT_MSGDEFS_H

#include "common_types.h"
#include "payuzut_fcncodes.h"


/************************************************
 * 
 * Telemetry Payload Type Definition
 * 
 ************************************************/
typedef struct PAYUZUT_HkTlm_Payload {
    
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

} PAYUZUT_HkTlm_Payload_t;

typedef struct PAYUZUT_BcnTlm_Payload {
    /**
     * Converted ADC value
     */
    uint16 ADC1; //1001001b
    uint16 ADC2; //1001000b
    
} PAYUZUT_BcnTlm_Payload_t;

#endif
