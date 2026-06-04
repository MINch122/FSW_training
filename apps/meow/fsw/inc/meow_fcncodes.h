/**
 * @file
 *   Specification for the MEOW command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef MEOW_FCNCODES_H
#define MEOW_FCNCODES_H

#include "meow_fcncode_values.h"

/* Housekeeping */
#define MEOW_NOOP_CC                    MEOW_CCVAL(NOOP)
#define MEOW_RESET_COUNTERS_CC          MEOW_CCVAL(RESET_COUNTERS)

/* Shell */
#define MEOW_SHELL_EXEC_SYNC_CC         MEOW_CCVAL(SHELL_EXEC_SYNC)
#define MEOW_SHELL_EXEC_ASYNC_CC        MEOW_CCVAL(SHELL_EXEC_ASYNC)
#define MEOW_SHELL_POLL_CC              MEOW_CCVAL(SHELL_POLL)
#define MEOW_SHELL_KILL_CC              MEOW_CCVAL(SHELL_KILL)
#define MEOW_SHELL_EXEC_SYNC_FORCE_CC   MEOW_CCVAL(SHELL_EXEC_SYNC_FORCE)
#define MEOW_SHELL_EXEC_ASYNC_FORCE_CC  MEOW_CCVAL(SHELL_EXEC_ASYNC_FORCE)

/* File */
#define MEOW_FILE_READ_CC               MEOW_CCVAL(FILE_READ)
#define MEOW_FILE_WRITE_CC              MEOW_CCVAL(FILE_WRITE)
#define MEOW_FILE_REMOVE_CC             MEOW_CCVAL(FILE_REMOVE)
#define MEOW_FILE_COPY_CC               MEOW_CCVAL(FILE_COPY)
#define MEOW_FILE_MOVE_CC               MEOW_CCVAL(FILE_MOVE)
#define MEOW_FILE_STAT_CC               MEOW_CCVAL(FILE_STAT)
#define MEOW_FILE_TRUNCATE_CC           MEOW_CCVAL(FILE_TRUNCATE)
#define MEOW_FILE_TAIL_CC               MEOW_CCVAL(FILE_TAIL)
#define MEOW_FILE_CHECKSUM_CC           MEOW_CCVAL(FILE_CHECKSUM)
#define MEOW_DISK_STAT_CC               MEOW_CCVAL(DISK_STAT)

/* Sys */
#define MEOW_SYS_SHUTDOWN_CC            MEOW_CCVAL(SYS_SHUTDOWN)
#define MEOW_SYS_SYNC_CC                MEOW_CCVAL(SYS_SYNC)
#define MEOW_SYS_INFO_CC                MEOW_CCVAL(SYS_INFO)
#define MEOW_SYS_TIME_GET_CC            MEOW_CCVAL(SYS_TIME_GET)
#define MEOW_SYS_TIME_SET_CC            MEOW_CCVAL(SYS_TIME_SET)
#define MEOW_SYS_FORCE_KILL_CC          MEOW_CCVAL(SYS_FORCE_KILL)

/* CSP */
#define MEOW_CSP_SERVER_START_CC        MEOW_CCVAL(CSP_SERVER_START)
#define MEOW_CSP_SERVER_STOP_CC         MEOW_CCVAL(CSP_SERVER_STOP)
#define MEOW_CSP_SET_READ_TIMEOUT_CC    MEOW_CCVAL(CSP_SET_READ_TIMEOUT)
#define MEOW_CSP_HANDLER_LOAD_CC        MEOW_CCVAL(CSP_HANDLER_LOAD)
#define MEOW_CSP_HANDLER_CLEAR_CC       MEOW_CCVAL(CSP_HANDLER_CLEAR)
#define MEOW_CSP_SEND_CC                MEOW_CCVAL(CSP_SEND)
#define MEOW_CSP_FTP_UPLOAD_CC          MEOW_CCVAL(CSP_FTP_UPLOAD)
#define MEOW_CSP_FTP_DOWNLOAD_CC        MEOW_CCVAL(CSP_FTP_DOWNLOAD)
#define MEOW_CSP_IFSTATS_CC             MEOW_CCVAL(CSP_IFSTATS)
#define MEOW_CSP_ROUTE_SET_CC           MEOW_CCVAL(CSP_ROUTE_SET)
#define MEOW_CSP_REROUTE_SET_CC         MEOW_CCVAL(CSP_REROUTE_SET)
#define MEOW_CSP_REROUTE_CLEAR_CC       MEOW_CCVAL(CSP_REROUTE_CLEAR)

#endif /* MEOW_FCNCODES_H */
