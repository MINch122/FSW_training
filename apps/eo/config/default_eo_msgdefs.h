#ifndef EO_MSGDEFS_H
#define EO_MSGDEFS_H

#include "common_types.h"
#include "eo_fcncodes.h"


typedef struct EO_HkTlm_Payload{
    
    uint16_t TBD1;
    uint16_t TBD2;
    uint8_t TBD3;
    
    uint8_t CmdCounter;
    uint8_t CmdErrCounter;

}__attribute__((packed)) EO_BcnTlm_Payload_t;

#endif