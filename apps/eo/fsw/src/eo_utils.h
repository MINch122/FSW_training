/**
 * @file
 *
 * Main header file for the EO util function
 */

#ifndef EO_UTILS_H
#define EO_UTILS_H

#include "common_types.h"
#include "eo_mission_cfg.h"

#include "eps_msgids.h"
#include "eps_msg.h"
#include "eps_interface_cfg.h"

// #include "sp_msgids.h"
// #include "sp_msg.h"

#include "sc_msgids.h"
#include "sc_msg.h"

#include "rpt_msgids.h"
#include "rpt_msg.h"

#include "adcs_msgids.h"
#include "adcs_msg.h"

#include "to_lab_msgids.h"
#include "to_lab_msg.h"

#include "hk_msgids.h"
#include "hk_msg.h"

#include "uant_app_msgids.h"
#include "uant_app_msg.h"

/* Debug definition */
#ifdef EO_DEBUG
#define EO_PRINTF(...)  OS_printf(__VA_ARGS__)
#else
#define EO_PRINTF(...)
#endif


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} EO_ReportTlm_t;


void EO_RequestVbattEPS(void);
void EO_RequestSetOutSingle(uint8_t Channel, uint8_t Value);
void EO_RequestOutEPS(void);

void EO_ChildTask(void);
void EO_ExitApps(void);

// void EO_SantDeploy(void);
void EO_SantConfirm(void);

void EO_TCWait(void);

void EO_EnableTO(void);

/**
 * @brief TO Subscribe The HK combined Packet 1 (Beacon, `0x081A)`
 */
void EO_EnableBeacon(void);
/**
 * @brief TO "Un" Subscribe The HK combined Packet 1 (Beacon, `0x081A)`
 */
void EO_DisableBeacon(void);

void EO_SPDeploy(CFE_SRL_GPIO_Handle_t *Handle, uint8_t Duration);

void EO_RequestMMTDeploy(void);
void EO_RequestMMTTlm(void);

void EO_AdcsDetumble(void);

void EO_EnableRTS2(void);
void EO_StartRTS2(void);



void EO_FinalizePhase(void);

#endif