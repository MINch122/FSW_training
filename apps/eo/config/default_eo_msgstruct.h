#ifndef EO_MSGSTRUCT_H
#define EO_MSGSTRUCT_H

#include "eo_mission_cfg.h"
#include "eo_msgdefs.h"
#include "cfe_msg_hdr.h"



typedef struct {

    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    EO_BcnTlm_Payload_t Payload;
    
} EO_BcnTlm_t;


typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;

} EO_NoopCmd_t;

typedef struct {
    
    CFE_MSG_CommandHeader_t CommandHeader;

} EO_ResetCounterCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;

} EO_ResetPhaseCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;

} EO_NextPhaseCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} EO_FinishPhaseCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} EO_ExitChildTaskCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} EO_StartChildTaskCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} EO_AppsPermOffCmd_t;



#endif