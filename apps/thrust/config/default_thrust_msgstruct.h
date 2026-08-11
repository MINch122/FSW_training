/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
 *
 * Licensed under the Apache License, Version 2.0
 ************************************************************************/

/**
 * @file
 *   Specification for the THRUST command and telemetry message data types.
 */

#ifndef THRUST_MSGSTRUCT_H
#define THRUST_MSGSTRUCT_H

#include "thrust_msgdefs.h"

#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

/*
** The following commands all share the "NoArgs" format.
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the
** prototype of the handler function.
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_SendHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_ReqStatusCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_PingCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_DisarmCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_MainAbortCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_CGAbortCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_ReqFaultLogCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} THRUST_ClearFaultCmd_t;

/********************************************************
 *
 * iG4U command structures
 *
 ********************************************************/
typedef struct
{
    CFE_MSG_CommandHeader_t      CommandHeader;
    THRUST_ResetModule_Payload_t Payload;
} THRUST_ResetModuleCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t  CommandHeader;
    THRUST_SetMode_Payload_t Payload;
} THRUST_SetModeCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    THRUST_Arm_Payload_t    Payload;
} THRUST_ArmCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t   CommandHeader;
    THRUST_MainFire_Payload_t Payload;
} THRUST_MainFireCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t  CommandHeader;
    THRUST_CGPulse_Payload_t Payload;
} THRUST_CGPulseCmd_t;

/*************************************************************************/
/*
** Type definition (Telemetry)
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    THRUST_HkTlm_Payload_t    Payload;
} THRUST_HkTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    THRUST_StatusTlm_Payload_t Payload;
} THRUST_StatusTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Report;
} THRUST_ReportTlm_t;

#endif /* THRUST_MSGSTRUCT_H */
