/**
 * @file
 *
 * MEOW Application Platform Configuration Header File
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
#ifndef MEOW_INTERNAL_CFG_H
#define MEOW_INTERNAL_CFG_H

#include "meow_internal_cfg_values.h"

/** Depth of the software bus command pipe. */
#define MEOW_PLATFORM_PIPE_DEPTH         MEOW_PLATFORM_CFGVAL(PIPE_DEPTH)
#define DEFAULT_MEOW_PLATFORM_PIPE_DEPTH 32

/** Name of the software bus command pipe. */
#define MEOW_PLATFORM_PIPE_NAME         MEOW_PLATFORM_CFGVAL(PIPE_NAME)
#define DEFAULT_MEOW_PLATFORM_PIPE_NAME "MEOW_CMD_PIPE"

#endif /* MEOW_INTERNAL_CFG_H */
