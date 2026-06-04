/**
 * @file
 *   MEOW Application Public Definitions
 *
 * This provides default values for configurable items that affect
 * the interface(s) of this module.  This includes the CMD/TLM message
 * interface and any other data products that serve to exchange
 * information with other entities.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef DEFAULT_MEOW_INTERFACE_CFG_H
#define DEFAULT_MEOW_INTERFACE_CFG_H

/* Use the default configuration value for all */
#define MEOW_MISSION_CFGVAL(x) DEFAULT_MEOW_MISSION_##x

#endif /* DEFAULT_MEOW_INTERFACE_CFG_H */
