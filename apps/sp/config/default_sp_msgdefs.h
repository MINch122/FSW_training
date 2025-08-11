#ifndef SP_APP_MSGDEFS_H
#define SP_APP_MSGDEFS_H

#include "sp_interface_cfg.h"
#include "sp_fcncodes.h"
#include "common_types.h"

typedef struct SP_APP_Deploy_Payload{
    uint8_t deploy;
} SP_APP_Deploy_Payload_t;

typedef struct SP_APP_DEPTlm_Payload{
    uint8_t get_result;
} SP_APP_DEPTlm_Payload_t;
#endif