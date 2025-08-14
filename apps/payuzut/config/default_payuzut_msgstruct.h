#ifndef PAYUZUT_MSGSTRUCT_H
#define PAYUZUT_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "payuzut_mission_cfg.h"
#include "payuzut_msgdefs.h"
#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

/*************************************************************************/

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_ResetCountersCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_GetTempCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_ThrusterOnCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_ThrusterOffCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_CumulateTempCmd_t;


/*************************************************************************/
/*
** Type definition (PAY UZURO THRUSTER App housekeeping)
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_SendHkCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUT_SendBcnCmd_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAYUZUT_HkTlm_Payload_t Payload;
} PAYUZUT_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAYUZUT_BcnTlm_Payload_t Payload;
} PAYUZUT_BcnTlm_t;


/**
 * Report Tlm for RPT
 */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} PAYUZUT_ReportTlm_t;


#endif
