/**
 * @file
 *
 * MEOW Application Mission Configuration Header File
 *
 * This is a compatibility header for the "mission_cfg.h" file that has
 * traditionally provided public config definitions for each CFS app.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef MEOW_INTERFACE_CFG_H
#define MEOW_INTERFACE_CFG_H

#include "meow_interface_cfg_values.h"

/** Maximum shell command string length including NUL terminator. */
#define MEOW_MISSION_MAX_CMD_LEN           MEOW_MISSION_CFGVAL(MAX_CMD_LEN)
#define DEFAULT_MEOW_MISSION_MAX_CMD_LEN   80

/** Maximum file or directory path length including NUL terminator. */
#define MEOW_MISSION_MAX_PATH_LEN          MEOW_MISSION_CFGVAL(MAX_PATH_LEN)
#define DEFAULT_MEOW_MISSION_MAX_PATH_LEN  80

/** Maximum payload bytes per FILE_WRITE command. */
#define MEOW_MISSION_MAX_WRITE_LEN         MEOW_MISSION_CFGVAL(MAX_WRITE_LEN)
#define DEFAULT_MEOW_MISSION_MAX_WRITE_LEN 80

/** Maximum bytes returned in a read/tail/checksum telemetry packet. */
#define MEOW_MISSION_MAX_READ_LEN          MEOW_MISSION_CFGVAL(MAX_READ_LEN)
#define DEFAULT_MEOW_MISSION_MAX_READ_LEN  450

/** Maximum bytes carried in report telemetry, matching the RPT app report ABI. */
#define MEOW_MISSION_MAX_REPORT_LEN         MEOW_MISSION_CFGVAL(MAX_REPORT_LEN)
#define DEFAULT_MEOW_MISSION_MAX_REPORT_LEN 512

/** Maximum CSP interface name length including NUL terminator. */
#define MEOW_MISSION_MAX_IFACE_NAME_LEN          MEOW_MISSION_CFGVAL(MAX_IFACE_NAME_LEN)
#define DEFAULT_MEOW_MISSION_MAX_IFACE_NAME_LEN  16

/** Maximum handler symbol name length including NUL terminator (CSP handler load). */
#define MEOW_MISSION_MAX_SYMBOL_LEN              MEOW_MISSION_CFGVAL(MAX_SYMBOL_LEN)
#define DEFAULT_MEOW_MISSION_MAX_SYMBOL_LEN      64

/**
 * Magic word required in the SYS_SHUTDOWN command payload.
 * Interpreted as big-endian regardless of host byte order: 0x90 0x0D 0xB0 0x07.
 */
#define MEOW_MISSION_SYS_SHUTDOWN_MAGIC    MEOW_MISSION_CFGVAL(SYS_SHUTDOWN_MAGIC)
#define DEFAULT_MEOW_MISSION_SYS_SHUTDOWN_MAGIC 0x900DB007u

/**
 * Magic word required in SHELL_EXEC_*_FORCE command payloads.
 * Interpreted as big-endian regardless of host byte order: 0xDE 0xAD 0xC0 0xDE.
 */
#define MEOW_MISSION_SHELL_FORCE_MAGIC     MEOW_MISSION_CFGVAL(SHELL_FORCE_MAGIC)
#define DEFAULT_MEOW_MISSION_SHELL_FORCE_MAGIC 0xDEADC0DEu

/**
 * Magic word required in SYS_FORCE_KILL command payloads.
 * Interpreted as big-endian regardless of host byte order: 0xDE 0xAD 0xFA 0xCE.
 */
#define MEOW_MISSION_SYS_FORCE_KILL_MAGIC  MEOW_MISSION_CFGVAL(SYS_FORCE_KILL_MAGIC)
#define DEFAULT_MEOW_MISSION_SYS_FORCE_KILL_MAGIC 0xDEADFACEu

#endif /* MEOW_INTERFACE_CFG_H */
