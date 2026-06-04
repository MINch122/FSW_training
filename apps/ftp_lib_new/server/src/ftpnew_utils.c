/**
 * @file ftpnew_utils.c
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief FTP-NEW server utility functions.
 * @version 0.1
 * @date 2026-06-01
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#include "ftpnew.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_WHITE   "\x1b[37m"

/* Per-thread "last debug message" buffer. Sized to match the log entry's
 * message field so a verbatim copy always fits. */
static __thread char tls_last_msg[FTP_LOG_MESSAGE_LENGTH];

static const char* level_color(ftp_log_level_t level)
{
    switch (level) {
        case FTP_LOG_ERROR:
            return COLOR_RED;
        case FTP_LOG_WARNING:
            return COLOR_YELLOW;
        case FTP_LOG_INFO:
            return COLOR_GREEN;
        case FTP_LOG_NORMAL:
        default:
            return COLOR_WHITE;
    }
}

void ftp_debug_impl(ftp_log_level_t level, const char* func, const char* fmt, ...)
{
    /* Always capture into the per-thread buffer (regardless of build mode)
     * so ftp_append_result has something to attach to the log entry. */
    va_list va;
    va_start(va, fmt);
    int n = vsnprintf(tls_last_msg, sizeof(tls_last_msg), fmt, va);
    va_end(va);
    if (n < 0)
        tls_last_msg[0] = 0;

#if defined(FTPNEW_DEBUG) && FTPNEW_DEBUG
    /* Strip a trailing newline so the color reset lands at the end of the
     * visible text; emit our own newline after the reset. */
    
    size_t msg_len = strlen(tls_last_msg);
    if (msg_len > 0 && tls_last_msg[msg_len - 1] == '\n')
        msg_len--;

    fprintf(stderr, "%s%s: %.*s%s\n",
            level_color(level),
            func ? func : "(no-func)",
            (int)msg_len, tls_last_msg,
            COLOR_RESET);
    fflush(stderr);
#else
    (void)level;
    (void)func;
#endif
}

const char* ftp_debug_get_last_message(void)
{
    return tls_last_msg;
}

void ftp_debug_clear_last_message(void)
{
    tls_last_msg[0] = 0;
}

uint32_t ftp_crc32_init(void)
{
    return 0xFFFFFFFFu;
}

uint32_t ftp_crc32_update(uint32_t crc, const void *buf, size_t len)
{
    const uint8_t *p = (const uint8_t *)buf;
    while (len--) {
        crc ^= *p++;
        for (int i = 0; i < 8; ++i)
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xEDB88320u : 0u);
    }
    return crc;
}

uint32_t ftp_crc32_finalize(uint32_t crc)
{
    return ~crc;
}
