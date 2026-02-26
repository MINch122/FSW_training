#ifndef SP_MSGDEFS_H
#define SP_MSGDEFS_H

#include "sp_interface_cfg.h"
#include "sp_fcncodes.h"
#include "common_types.h"

/**
 * AR6 release status for one burn-wire channel.
 * Mirrors gs_gssb_ar6_release_status_t from <gs/gssb/gssb.h>.
 */
typedef struct SP_AR6_Status_Payload
{
    uint8 BurnState;      /**< Burn state:      1=Burning, 0=Idle     */
    uint8 ReleaseStatus;  /**< Release status:  1=Released, 0=Held    */
    uint8 BurnTimeLeft;   /**< Burn time remaining [s]                */
    uint8 BurnTries;      /**< Cumulative burn attempt counter        */
} SP_AR6_Status_Payload_t;

/**
 * Per-DSP telemetry payload.
 * One DSP board has two AR6 devices (Board A and Board B).
 */
typedef struct SP_DSP_Tlm_Payload
{
    SP_AR6_Status_Payload_t BoardA; /**< AR6 Board A release status */
    SP_AR6_Status_Payload_t BoardB; /**< AR6 Board B release status */
    int16  TempA;                   /**< Board A internal temp [ddegC] */
    int16  TempB;                   /**< Board B internal temp [ddegC] */
    uint32 SecondsSinceBoot;        /**< Board uptime [s] */
    uint8  RebootCount;             /**< Reboot counter */
    uint8  spare[3];
} SP_DSP_Tlm_Payload_t;

/**
 * Full HK telemetry payload (both DSP boards).
 */
typedef struct SP_HkTlm_Payload
{
    uint8  CommandCounter;
    uint8  CommandErrorCounter;
    uint8  spare[2];
    SP_DSP_Tlm_Payload_t Dsp[2]; /**< Index 0=DSP1 (SP1), 1=DSP2 (SP2) */
} SP_HkTlm_Payload_t;

/**
 * Deploy command payload - select which DSP to burn and for how long.
 */
typedef struct SP_Deploy_Payload
{
    uint8 DspNum;       /**< Target DSP: 0=DSP1 (SP1), 1=DSP2 (SP2) */
    uint8 BurnDuration; /**< Burn duration [s]; 0 = use default (SP_DSP_BURN_DURATION_S) */
    uint8 spare[2];
} SP_Deploy_Payload_t;

/**
 * Auto deploy command payload - deploy both DSPs with retry logic.
 * Uses gs_autodeploy_release_two_dsp() internally.
 * Set all fields to 0 to use defaults from SP_INTERFACE_CFG.
 */
typedef struct SP_AutoDeploy_Payload
{
    uint8 StartBurnTime; /**< Initial burn duration [s]; 0 = use default */
    uint8 Increment;     /**< Burn time increment per retry [s]; 0 = use default */
    uint8 MaxBurnTime;   /**< Maximum burn duration [s]; 0 = use default */
    uint8 spare;
} SP_AutoDeploy_Payload_t;

/**
 * Stop burn command payload.
 */
typedef struct SP_StopBurn_Payload
{
    uint8 DspNum; /**< Target DSP: 0=DSP1, 1=DSP2 */
    uint8 spare[3];
} SP_StopBurn_Payload_t;

/**
 * Beacon telemetry payload - lightweight deploy status for housekeeping.
 */
typedef struct SP_BcnTlm_Payload
{
    uint8 DeployStatus[2]; /**< Index 0=DSP1, 1=DSP2. 1=Released, 0=Not released */
} SP_BcnTlm_Payload_t;

/**
 * Set AR6 I2C address command payload.
 * Programs a new I2C address into an AR6 board and commits it to NVM.
 * Used during hardware commissioning when GOSH is not available.
 * CAUTION: Connect only ONE AR6 board to the bus before sending this command.
 */
typedef struct SP_SetAr6Addr_Payload
{
    uint8 CurrentAddr; /**< Current I2C address of the target AR6 board */
    uint8 NewAddr;     /**< New I2C address to program into NVM */
    uint8 spare[2];
} SP_SetAr6Addr_Payload_t;

#endif