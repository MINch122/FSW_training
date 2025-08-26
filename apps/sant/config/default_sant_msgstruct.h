/************************************************************************
 * SANT (GomSpace ANT‑6F) ‑ cFS Application
 *
 * Message Structure Definitions
 ************************************************************************/

#ifndef SANT_MSGSTRUCT_H
#define SANT_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include <stdint.h>

#include "cfe_msg_hdr.h"
#include "sant_msgdefs.h"
#include "sant_mission_cfg.h"

#include "rpt_interface_cfg.h"


/************************************************************************
 *  Command Messages
 *
 *  NOTE
 *  —— All commands that act on the ANT‑6F board carry the I²C address
 *     as the **first** payload byte (Addr).  Multi‑byte numerical
 *     fields are little‑endian by cFS.
 ************************************************************************/

/* 0‑argument commands */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
} SANT_NoopCmd_t;              /* FC = SANT_NOOP_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
} SANT_ResetCountersCmd_t;     /* FC = SANT_RESET_COUNTERS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t Addr;
} SANT_SoftRebootCmd_t;        /* FC = SANT_SOFT_REBOOT_CC */

/* Burn for N seconds
   Args: Addr, Duration(0‑60 s) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
    uint8_t                 Duration;
} SANT_BurnCmd_t;       /* FC = SANT_BURN_CC */

/* Stop any on‑going burn on the board
   Args: Addr */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_StopBurnCmd_t;          /* FC = SANT_STOP_BURN_CC */

/* Telemetry‑pull commands (all 1‑byte Addr argument) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_GetStatusCmd_t;         /* FC = SANT_GET_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_GetBackupStatusCmd_t;   /* FC = SANT_GET_BACKUP_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_GetBoardStatusCmd_t;    /* FC = SANT_GET_BOARD_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_GetTemperatureCmd_t;    /* FC = SANT_GET_TEMPERATURE_CC */

/* Backup‑deploy settings (read / write) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_GetSettingsCmd_t;       /* FC = SANT_GET_SETTINGS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
    uint16_t                MinutesUntilDeploy;   /* 0‑5000 min */
    uint8_t                 BackupActive;         /* 0|1 */
    uint8_t                 MaxBurnDuration;      /* 0‑127 s */
} SANT_SetSettingsCmd_t;       /* FC = SANT_SET_SETTINGS_CC */

/************************************************************************
 *  Telemetry Messages
 ************************************************************************/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SANT_SendHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SANT_SendBcnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SANT_SendOpCmd_t;


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SANT_OperationTlm_Payload_t Payload;
} SANT_OperationTlm_t;


typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SANT_HkTlm_Payload_t  Payload;
} SANT_HkTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SANT_BcnTlm_Payload_t Payload;
} SANT_BcnTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SANT_ReportTlm_t;

#endif /* SANT_MSGSTRUCT_H */
