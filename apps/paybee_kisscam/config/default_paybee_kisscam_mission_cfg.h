/**
 * @file
 *
 * PAY UZURO CAM Application Mission Configuration Header File
 *
 * This is a compatibility header for the "mission_cfg.h" file that has
 * traditionally provided public config definitions for each CFS app.
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef paybee_kisscam_MISSION_CFG_H
#define paybee_kisscam_MISSION_CFG_H

#include "paybee_kisscam_interface_cfg.h"

#ifndef PAYBEE_KISSCAM_DEBUG
#define PAYBEE_KISSCAM_DEBUG false
#endif

#ifndef DEBUG_PAYBEE_KISSCAM
#define DEBUG_PAYBEE_KISSCAM PAYBEE_KISSCAM_DEBUG
#endif

#if DEBUG_PAYBEE_KISSCAM
#define PAYBEE_KISSCAM_APP_printf(...) OS_printf(__VA_ARGS__)
#else
#define PAYBEE_KISSCAM_APP_printf(...) do { if (false) { OS_printf(__VA_ARGS__); } } while (0)
#endif

#endif
