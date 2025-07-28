#ifndef RPT_MSGSTRUCT_H
#define RPT_MSGSTRUCT_H

#include "rpt_mission_cfg.h"
#include "rpt_msgdefs.h"
#include "cfe_msg_hdr.h"



typedef struct {

    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_HkTlm_Payload_t Payload;
    
} RPT_HkTlm_t;


typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;

} RPT_NoopCmd_t;

typedef struct {
    
    CFE_MSG_CommandHeader_t CommandHeader;

} RPT_ResetCounterCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;
    RPT_Report_Payload_t Payload;

} RPT_ReportCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;
    RPT_ClearQueue_Payload_t Payload;

} RPT_ClearQueueCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} RPT_GetOpsDataCmd_t;


#endif