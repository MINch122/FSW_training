#ifndef RPT_MSGSTRUCT_H
#define RPT_MSGSTRUCT_H

#include "rpt_mission_cfg.h"
#include "rpt_msgdefs.h"
#include "cfe_msg_hdr.h"



typedef struct {

    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_BcnTlm_Payload_t Payload;
    
} RPT_BcnTlm_t;


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



typedef struct {
    uint32 Seconds;
    uint32 MicroSeconds;
} RPT_SetTimeCmd_Payload_t;

/*************************************************************************
 * Command Struct sended to cFE TIME service
 * **cFE TIME Set Time cmd**
 * Only used for RPT Init
 * ------------NOTE------------
 * # Should convert Subsecond to MicroSecond
 * The stored time value is `Seconds` and `SubSeconds`
 * But, `CFE_TIME_SetTimeCmd` needs `Seconds` and `MicroSeconds`
 * So, value should be converted by `CFE_TIME_Sub2MicroSecs()` API
 ************************************************************************/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    RPT_SetTimeCmd_Payload_t Payload;
} RPT_SetTimeCmd_t;


#endif