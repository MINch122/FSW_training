#ifndef PAYUEL_CAM_MSGDEFS_H
#define PAYUEL_CAM_MSGDEFS_H

#include "common_types.h"
#include "payuel_cam_fcncodes.h"

typedef struct PAYUEL_CAM_DisplayParam_Payload
{
    uint32 ValU32;
    int16  ValI16;
    char   ValStr[PAYUEL_CAM_STRING_VAL_LEN];
} PAYUEL_CAM_DisplayParam_Payload_t;

typedef struct PAYUEL_CAM_HkTlm_Payload
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 spare[2];
} PAYUEL_CAM_HkTlm_Payload_t;

#endif
