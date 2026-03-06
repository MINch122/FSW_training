/************************************************************************
 * UANT (GomSpace ANT‑6F) ‑ cFS Application
 *
 * Message Structure Definitions
 ************************************************************************/

#ifndef UANT_APP_MSGSTRUCT_H
#define UANT_APP_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include <stdint.h>


#include "cfe_msg_hdr.h"
#include "uant_app_msgdefs.h"      /* 기능‑코드, 길이 매크로 등 */
#include "uant_app_mission_cfg.h"  /* 미션별 상수 */


/* 0‑argument commands */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
} UANT_APP_NoopCmd_t;              /* FC = UANT_APP_NOOP_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
} UANT_APP_ResetCountersCmd_t;     /* FC = UANT_APP_RESET_COUNTERS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8 Addr;
} UANT_APP_SoftRebootCmd_t;        /* FC = UANT_APP_SOFT_REBOOT_CC */

/* Burn one channel for N seconds
   Args: Addr, Channel(0|1), Duration(0‑60 s) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                 Addr;
    uint8                 Channel;
    uint8                 Duration;
} UANT_APP_BurnChannelCmd_t;       /* FC = UANT_APP_BURN_CHANNEL_CC */

/* Stop any on‑going burn on the board
   Args: Addr */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                 Addr;
} UANT_APP_StopBurnCmd_t;          /* FC = UANT_APP_STOP_BURN_CC */

/* Telemetry‑pull commands (all 1‑byte Addr argument) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                 Addr;
} UANT_APP_GetStatusCmd_t;         /* FC = UANT_APP_GET_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                 Addr;
} UANT_APP_GetBackupStatusCmd_t;   /* FC = UANT_APP_GET_BACKUP_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                 Addr;
} UANT_APP_GetBoardStatusCmd_t;    /* FC = UANT_APP_GET_BOARD_STATUS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                Addr;
} UANT_APP_GetTemperatureCmd_t;    /* FC = UANT_APP_GET_TEMPERATURE_CC */

/* Backup‑deploy settings (read / write) */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                Addr;
} UANT_APP_GetSettingsCmd_t;       /* FC = UANT_APP_GET_SETTINGS_CC */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint8                 Addr;
    uint16                MinutesUntilDeploy;   /* 0‑5000 min */
    uint8                 BackupActive;         /* 0|1 */
    uint8                 MaxBurnDuration;      /* 0‑127 s */
} UANT_APP_SetSettingsCmd_t;       /* FC = UANT_APP_SET_SETTINGS_CC */

/* AUTODEPLOY */

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHdr;
    uint16 SecondsDelay;   /* 부팅 후 대기 시간(s)  */
    uint8  AddrAnt6_0;     /* 첫 번째 ANT‑6 I2C 주소 */
    uint8  AddrAnt6_1;     /* 두 번째 ANT‑6 I2C 주소 */
} UANT_APP_AutodeployCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} UANT_APP_SendBcnCmd_t;


/************************************************************************
 *  Telemetry Messages
 ************************************************************************/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} UANT_APP_SendHkCmd_t;


typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    /* -------- Board A (I2C 0x05) -------- */
    /* priority 0 */
    uint8  ch0_status_A;        /* release_status.channel_0_status */
    uint8  ch1_status_A;        /* release_status.channel_1_status */
    uint8  backup_active_A;     /* backup_settings.backup_active   */
    uint8  state_A;             /* backup_status.state             */
    /* priority 1 */
    uint8  ch0_burn_tries_A;    /* release_status.channel_0_burn_tries */
    uint8  ch1_burn_tries_A;    /* release_status.channel_1_burn_tries */
    uint8  reboot_count_A;      /* board_status.reboot_count           */

    /* -------- Board B (I2C 0x06) -------- */
    /* priority 0 */
    uint8  ch0_status_B;
    uint8  ch1_status_B;
    uint8  backup_active_B;
    uint8  state_B;
    /* priority 1 */
    uint8  ch0_burn_tries_B;
    uint8  ch1_burn_tries_B;
    uint8  reboot_count_B;

} UANT_APP_HkTlm_t;


typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    UANT_APP_BcnTlm_Payload_t Payload;

} UANT_APP_bcnTlm_t;


/*---------------- Release / Burn status ----------------
   Maps 1‑to‑1 to gs_gssb_ant6_get_release_status() payload */
typedef struct
{
    uint8 Ch0State;        /* 0 idle / 1 burning              */
    uint8 Ch0Released;     /* 0 not ‑ 1 yes                   */
    uint8 Ch0TimeLeft;     /* seconds                         */
    uint8 Ch0Tries;        /* burn attempts                   */
    uint8 Ch1State;
    uint8 Ch1Released;
    uint8 Ch1TimeLeft;
    uint8 Ch1Tries;
} UANT_APP_RlsStatus_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHdr;
    UANT_APP_RlsStatus_Payload_t Payload;
} UANT_APP_RlsStatusTlm_t;

/*---------------- Backup‑timer status ---------------
   gs_gssb_ant6_get_backup_status() */


typedef struct
{
    uint8  State;              /* 0‑4 per ANT‑6F spec */
    uint32 SecondsToDeploy;
} UANT_APP_BackupStatus_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHdr;
    UANT_APP_BackupStatus_Payload_t Payload;
} UANT_APP_BackupStatusTlm_t;

/*---------------- Board (MCU) status ----------------*/
typedef struct
{
    uint32 SecondsSinceBoot;
    uint8  RebootCount;
} UANT_APP_BoardStatus_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHdr;
    UANT_APP_BoardStatus_Payload_t Payload;
} UANT_APP_BoardStatusTlm_t;

/*---------------- Temperature -----------------------*/
typedef struct
{
    int16  Temperature; /* °C × 1 (LM75 style) */
} UANT_APP_Temp_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHdr;
    UANT_APP_Temp_Payload_t   Payload;
} UANT_APP_TempTlm_t;

/*---------------- Backup‑settings readback ----------*/
typedef struct
{
    uint16 MinutesUntilDeploy;
    uint8  BackupActive;
    uint8  MaxBurnDuration;
} UANT_APP_Settings_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHdr;
    UANT_APP_Settings_Payload_t Payload;
} UANT_APP_SettingsTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t              Payload;
} UANT_RPT_Tlm_t;


/* Command struct for request release status */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    UANT_RequestRelease_Payload_t Payload;
} UANT_RequestRelease_t;

/* Telemetry struct for release status */
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    UANT_InternalTlm_Payload_t Payload;
} UANT_InternalTlm_t;

#endif /* UANT_APP_MSGSTRUCT_H */
