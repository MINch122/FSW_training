/**
 * @file
 *   MEOW Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef DEFAULT_MEOW_INTERNAL_CFG_H
#define DEFAULT_MEOW_INTERNAL_CFG_H

/* Use the default configuration value for all */
#define MEOW_PLATFORM_CFGVAL(x) DEFAULT_MEOW_PLATFORM_##x

#endif /* DEFAULT_MEOW_INTERNAL_CFG_H */
