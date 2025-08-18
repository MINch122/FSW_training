/************************************************************************
 * SANT (GomSpace ANT‑6F) ‑ cFS Application
 *
 * Message Structure Definitions
 ************************************************************************/

#ifndef SANT_APP_MSGSTRUCT_H
#define SANT_APP_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include <stdint.h>

#include "cfe_msg_hdr.h"
#include "sant_app_msgdefs.h"
#include "sant_app_mission_cfg.h"


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
} SANT_APP_NoopCmd_t;              /* FC = SANT_APP_NOOP_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
} SANT_APP_ResetCountersCmd_t;     /* FC = SANT_APP_RESET_COUNTERS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t Addr;
} SANT_APP_SoftRebootCmd_t;        /* FC = SANT_APP_SOFT_REBOOT_CC */

/* Burn for N seconds
   Args: Addr, Duration(0‑60 s) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
    uint8_t                 Duration;
} SANT_APP_BurnCmd_t;       /* FC = SANT_APP_BURN_CC */

/* Stop any on‑going burn on the board
   Args: Addr */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_APP_StopBurnCmd_t;          /* FC = SANT_APP_STOP_BURN_CC */

/* Telemetry‑pull commands (all 1‑byte Addr argument) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_APP_GetStatusCmd_t;         /* FC = SANT_APP_GET_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_APP_GetBackupStatusCmd_t;   /* FC = SANT_APP_GET_BACKUP_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_APP_GetBoardStatusCmd_t;    /* FC = SANT_APP_GET_BOARD_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_APP_GetTemperatureCmd_t;    /* FC = SANT_APP_GET_TEMPERATURE_CC */

/* Backup‑deploy settings (read / write) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
} SANT_APP_GetSettingsCmd_t;       /* FC = SANT_APP_GET_SETTINGS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8_t                 Addr;
    uint16_t                MinutesUntilDeploy;   /* 0‑5000 min */
    uint8_t                 BackupActive;         /* 0|1 */
    uint8_t                 MaxBurnDuration;      /* 0‑127 s */
} SANT_APP_SetSettingsCmd_t;       /* FC = SANT_APP_SET_SETTINGS_CC */

/************************************************************************
 *  Telemetry Messages
 ************************************************************************/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SANT_APP_SendHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SANT_APP_SendBcnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} SANT_APP_SendOpCmd_t;


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SANT_APP_OperationTlm_Payload_t Payload;
} SANT_APP_OperationTlm_t;


typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SANT_APP_HkTlm_Payload_t  Payload;
} SANT_APP_HkTlm_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SANT_APP_BcnTlm_Payload_t Payload;
} SANT_APP_BcnTlm_t;


#endif /* SANT_APP_MSGSTRUCT_H */
