#ifndef SP_MSGSTRUCT_H
#define SP_MSGSTRUCT_H

#include "sp_mission_cfg.h"
#include "sp_msgdefs.h"
#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_SendBcnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    SP_Deploy_Payload_t Payload;
} SP_DeployCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_Get_DeployCmd_t;


typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SP_BcnTlm_Payload_t Payload;
} SP_BcnTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SP_ReportTlm_t;

#endif