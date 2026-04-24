#ifndef EO_MSGDEFS_H
#define EO_MSGDEFS_H

#include "common_types.h"
#include "eo_fcncodes.h"


typedef struct EO_HkTlm_Payload{
    uint8 CmdCounter;
    uint8 CmdErrCounter;

    EO_CurrentStep_t PhaseInfo;

}__attribute__((packed)) EO_BcnTlm_Payload_t;

#endif