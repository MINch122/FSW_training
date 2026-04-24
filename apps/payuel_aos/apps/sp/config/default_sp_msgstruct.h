#ifndef SP_MSGSTRUCT_H
#define SP_MSGSTRUCT_H

#include "sp_mission_cfg.h"
#include "sp_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

/* ---- Command messages ---- */

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
} SP_SendHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_GetHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_SendBcnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    SP_Deploy_Payload_t Payload;
} SP_DeployCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    SP_StopBurn_Payload_t Payload;
} SP_StopBurnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    SP_AutoDeploy_Payload_t Payload;
} SP_AutoDeployCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_ScanAr6Cmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SP_ReportBcnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    SP_SetAr6Addr_Payload_t Payload;
} SP_SetAr6AddrCmd_t;

/* ---- Telemetry messages ---- */

/** Full HK telemetry: detailed AR6 status for both DSPs */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SP_HkTlm_Payload_t Payload;
} SP_HkTlm_t;

/** Beacon telemetry: DSP status for 4 devices */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SP_BcnTlm_Payload_t Payload;
    bool  IsRunning;     /**< Burn task active flag */
    bool  IsDeploy[2];   /**< Deployed flag per DSP (AR6 pair) */
    uint8 MaxTry;        /**< Burn attempt counter */
    uint8 spare;
} SP_BcnTlm_t;

/** Report telemetry: command result forwarded to RPT app */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SP_ReportTlm_t;

#endif