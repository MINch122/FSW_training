#ifndef SP_MSGDEFS_H
#define SP_MSGDEFS_H

#include "sp_interface_cfg.h"
#include "sp_fcncodes.h"
#include "common_types.h"

typedef struct SP_Deploy_Payload{
    uint8_t SP;     /* <\brief Solar Panel number `0` or `1` */
    uint8_t deploy; /* <\brief GPIO logic level `0` for low, `1` for high */
} SP_Deploy_Payload_t;

typedef struct SP_DEPTlm_Payload{
    uint8_t DeployStatus;
} SP_BcnTlm_Payload_t;

#endif