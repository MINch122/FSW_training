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
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_StartDeployTaskCmd_t;


typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SP_BcnTlm_Payload_t Payload;

    /**
     * Other critical operation data to construct the deploy algorithm
     */
    bool IsRunning; // Flag which indicate the "Running status of deploy thread"
    bool IsDeploy;  // Flag which indicate the "Deployed status"
    uint8 MaxTry;   // Maximum auto try [TBD]
    
} SP_BcnTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SP_ReportTlm_t;

#endif