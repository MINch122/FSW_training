/**
 * @file ftpnew.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief FTP-NEW server.
 *        FTP-NEW stands for "F***ing Tired of Patching this 
 *            Nonsensical Extensions and Wrappers for FTP."
 * @version 0.1
 * @date 2026-06-01
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#ifndef _FTPNEW_H_
#define _FTPNEW_H_

#include "ftpnew_types.h"

#include <stdint.h>
#include <stddef.h>

#define FTPNEW_DEBUG  1

/* ftp_ret_t lives in config/ftpnew_types.h -- shared between server and
 * client. The server writes these values directly to the wire; the client
 * maps them to gs_error_t with no intermediate translation. */


/* ========================================================================
 *                   Main server thread launcher.
 * ====================================================================== */

/**
 * A single connection handler thread.
 */
void* ftp_handler_thread(void* arg);

/**
 * @brief Launch the FTP-NEW server.
 */
int ftp_server_start(void);


/* ========================================================================
 *                   Utility functions (logs / CRC)
 * ====================================================================== */

typedef enum {
    FTP_LOG_NORMAL  = 0,
    FTP_LOG_INFO    = 1,
    FTP_LOG_WARNING = 2,
    FTP_LOG_ERROR   = 3,
} ftp_log_level_t;

void ftp_debug_impl(ftp_log_level_t level, const char* func, const char* fmt, ...);

/**
 * Returns the per-thread most-recent debug message. The pointer remains
 * valid until the next ftp_debug_* call on the same thread.
 */
const char* ftp_debug_get_last_message(void);

/**
 * Clears the per-thread last-message buffer.
 */
void ftp_debug_clear_last_message(void);

#define ftp_debug_normal(...)  ftp_debug_impl(FTP_LOG_NORMAL,  __func__, __VA_ARGS__)
#define ftp_debug_info(...)    ftp_debug_impl(FTP_LOG_INFO,    __func__, __VA_ARGS__)
#define ftp_debug_warning(...) ftp_debug_impl(FTP_LOG_WARNING, __func__, __VA_ARGS__)
#define ftp_debug_error(...)   ftp_debug_impl(FTP_LOG_ERROR,   __func__, __VA_ARGS__)

/**
 * Append a log entry with the given return code, packet type, and the most
 * recent debug message. Not to be called from userspace.
 */
void ftp_append_result(ftp_ret_t ret, uint8_t type);

/* 
 * CRC32 implementation.
 */
uint32_t ftp_crc32_init(void);
uint32_t ftp_crc32_update(uint32_t crc, const void *buf, size_t len);
uint32_t ftp_crc32_finalize(uint32_t crc);

#endif
