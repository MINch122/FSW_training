/**
 * @file
 *
 * CFE Serial Services (CFE_SRL) Application Platform Configuration Header File
 *
 * This is a compatibility header for the "platform_cfg.h" file that has
 * traditionally provided both public and private config definitions
 * for each CFS app.
 *
 * These definitions are now provided in two separate files, one for
 * the public/mission scope and one for internal scope.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef CFE_SRL_PLATFORM_CFG_H
#define CFE_SRL_PLATFORM_CFG_H

#include "cfe_srl_mission_cfg.h"
#include "cfe_srl_internal_cfg.h"

#endif /* CFE_SRL_PLATFORM_CFG_H */