#ifndef MISSION_MSGSTRUCT_H
#define MISSION_MSGSTRUCT_H

#include "mission_mission_cfg.h"
#include "mission_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"



typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    MISSION_BcnTlm_Payload_t Payload;
} MISSION_BcnTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    MISSION_HkTlm_Payload_t Payload;
} MISSION_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} MISSION_ReportTlm_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MISSION_SendHkCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;

} MISSION_NoopCmd_t;

typedef struct {
    
    CFE_MSG_CommandHeader_t CommandHeader;

} MISSION_ResetCounterCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} MISSION_AppsPermOffCmd_t;


#endif
