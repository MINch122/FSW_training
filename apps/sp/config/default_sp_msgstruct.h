#ifndef SP_APP_MSGSTRUCT_H
#define SP_APP_MSGSTRUCT_H

#include "sp_mission_cfg.h"
#include "sp_msgdefs.h"
#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_APP_SendBcnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    SP_APP_Deploy_Payload_t Payload;
} SP_APP_DeployCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_APP_Get_DeployCmd_t;


typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SP_APP_DEPTlm_Payload_t Payload;
} SP_APP_DEPTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SP_APP_ReportTlm_t;

#endif