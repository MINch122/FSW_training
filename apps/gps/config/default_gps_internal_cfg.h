/**
 * @file  GPS application private config definitions
 *
 * Internal to this module; changing them is transparent to everything using
 * the GPS command and telemetry interface.
 */
#ifndef GPS_INTERNAL_CFG_H
#define GPS_INTERNAL_CFG_H

#define GPS_PLATFORM_PIPE_DEPTH         32          /* Command pipe depth */
#define GPS_PLATFORM_PIPE_NAME          "GPS_CMD_PIPE"

#define GPS_PLATFORM_IFACE_INDEX        0           /* oem_io interface slot */
#define GPS_PLATFORM_SERIAL_DEV         "/dev/ttyS4"
#define GPS_PLATFORM_SERIAL_BAUD        115200
#define GPS_PLATFORM_RX_BUF_SIZE        0           /* 0 = driver default */

/** Read window. Bounds shutdown latency only; logs arrive on their own schedule. */
#define GPS_PLATFORM_RX_TIMEOUT_MS      10000

/** Yield after any receive fault, so a persistent one cannot spin. */
#define GPS_PLATFORM_RX_BACKOFF_MS      100

/** Shutdown join budget. Must exceed one read window, or the device is left
 *  open rather than freed under a task still reading it. */
#define GPS_PLATFORM_RX_JOIN_MS         12000
#define GPS_PLATFORM_RX_JOIN_POLL_MS    100

#define GPS_PLATFORM_RX_TASK_NAME       "GPS_RX"
#define GPS_PLATFORM_RX_STACK_SIZE      16384
#define GPS_PLATFORM_RX_PRIORITY        110

/** Callback modules must live under this prefix. */
#define GPS_PLATFORM_MODULE_DIR         "/cf/"

#endif
