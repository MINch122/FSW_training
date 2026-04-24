#ifndef PAYUEL_OBC_MSGDEFS_H
#define PAYUEL_OBC_MSGDEFS_H

#include "common_types.h"
#include "payuel_obc_fcncodes.h"

typedef struct PAYUEL_OBC_DisplayParam_Payload
{
    uint32 ValU32;
    int16  ValI16;
    char   ValStr[PAYUEL_OBC_STRING_VAL_LEN];
} PAYUEL_OBC_DisplayParam_Payload_t;

typedef struct PAYUEL_OBC_HkTlm_Payload
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 spare[2];
} PAYUEL_OBC_HkTlm_Payload_t;

#endif
