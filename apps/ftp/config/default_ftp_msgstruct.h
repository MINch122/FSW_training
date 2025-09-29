/**
 * @file
 *   Specification for the FTP command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in ftpuzuc_msgdefs.h.
 */
#ifndef FTP_MSGSTRUCT_H
#define FTP_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "ftp_mission_cfg.h"
#include "ftp_msgdefs.h"
#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

/************************************************************************
 * CMD Structure
 ************************************************************************/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} FTP_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} FTP_ResetCountersCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    FTP_SendFileCmd_Payload_t Payload;
} FTP_SendFileCmd_t;


typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} FTP_SendHkCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} FTP_SendBcnCmd_t;

// Other Cmd struct...

/************************************************************************
 * TLM Structure
 ************************************************************************/
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    FTP_HkTlm_Payload_t Payload;
} FTP_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    FTP_BcnTlm_Payload_t Payload;
} FTP_BcnTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    FTP_File_Payload_t Payload;
} FTP_File_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} FTP_ReportTlm_t;


#endif