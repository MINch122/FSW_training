/**
 * @file esup_utils.h
 * @brief Compile-time-toggled debug logging for the ESUP driver.
 *
 * A tiny logger used on error paths. Features:
 *   - Compile-time master switch: define ESUP_LOG (non-zero) to enable; when off,
 *     every ESUP_LOG* call expands to nothing and its arguments are not evaluated.
 *   - Each line is prefixed with a level tag and the calling function name.
 *   - Four levels, coloured on a terminal: NORMAL (white), INFO (green),
 *     WARNING (yellow), ERROR (red). Colour is emitted only to a tty; a plain
 *     text file gets no escape codes.
 *   - Runtime per-level enable/disable (esup_log_enable).
 *   - A second, runtime-registered output stream (esup_log_set_file), so logs go
 *     to stderr and, e.g., a .txt file at the same time.
 *
 * Usage: ESUP_LOGE("bad reply status 0x%04X", status);
 */
#ifndef ESUP_UTILS_H
#define ESUP_UTILS_H

#include <stdbool.h>
#include <stdio.h>

/* The compile-time log switch (ESUP_LOG) lives in the central config header, so
 * every translation unit that logs — and this facility's own implementation —
 * sees one consistent setting. Define ESUP_LOG to non-zero there to enable. */
#include "esup_config.h"

/**
 * @brief Log levels, in increasing severity.
 *
 * PACKET is a separate class for on-wire hex dumps (esup_log_hex); it is not a
 * severity but shares the same runtime enable/disable machinery.
 */
typedef enum {
    ESUP_LOG_NORMAL  = 0, /**< White. */
    ESUP_LOG_INFO    = 1, /**< Green. */
    ESUP_LOG_WARNING = 2, /**< Yellow. */
    ESUP_LOG_ERROR   = 3, /**< Red. */
    ESUP_LOG_PACKET  = 4  /**< White; on-wire packet hex dumps. */
} esup_log_level_t;

/* Packet hex-dump layout (compile-time). Bytes are single-space separated. */
#define ESUP_LOG_HEX_GROUP 8u   /**< Use a double space after this many bytes. */
#define ESUP_LOG_HEX_ROW   16u  /**< Insert a newline after this many bytes. */
#define ESUP_LOG_HEX_MAX   128u /**< Beyond this many bytes, elide the middle with "...". */

/**
 * @brief Enable or disable a specific level at runtime (all enabled by default).
 *
 * @param level   The level to toggle.
 * @param enabled Whether messages at @p level are emitted.
 */
void esup_log_enable(esup_log_level_t level, bool enabled);

/**
 * @brief Register an additional output stream besides stderr.
 *
 * The logger always writes to stderr; if @p stream is non-NULL it is also
 * written (colour is applied per stream based on whether it is a tty). The
 * caller owns the stream and must keep it open while registered.
 *
 * @param stream A writable stream (e.g. an fopen'd .txt file), or NULL to clear.
 */
void esup_log_set_file(FILE* stream);

/**
 * @brief Emit one log line. Prefer the ESUP_LOG* macros, which inject __func__.
 *
 * @param level The message level.
 * @param func  Calling function name.
 * @param fmt   printf-style format.
 * @param ...   Format arguments.
 */
void esup_log_write(esup_log_level_t level, const char* func, const char* fmt,
                    ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 3, 4)))
#endif
    ;

/**
 * @brief Emit a hex dump of a byte buffer (the PACKET class). Prefer ESUP_LOG_HEX.
 *
 * Layout: two hex digits per byte, a single space between bytes, a double space
 * at every ESUP_LOG_HEX_GROUP-byte boundary, a newline at every
 * ESUP_LOG_HEX_ROW-byte boundary. If @p len exceeds ESUP_LOG_HEX_MAX, the first
 * and last ESUP_LOG_HEX_MAX/2 bytes are shown with "..." between them.
 *
 * @param level Log level (ESUP_LOG_PACKET).
 * @param func  Calling function name.
 * @param tag   Short label for the dump (e.g. "TX", "RX"), or NULL.
 * @param data  Bytes to dump.
 * @param len   Number of bytes.
 */
void esup_log_hex(esup_log_level_t level, const char* func, const char* tag,
                  const void* data, size_t len);

#if defined(ESUP_LOG) && (ESUP_LOG)
#  define ESUP_LOG_AT(level, ...) esup_log_write((level), __func__, __VA_ARGS__)
#  define ESUP_LOG_HEX(tag, data, len) \
       esup_log_hex(ESUP_LOG_PACKET, __func__, (tag), (data), (len))
#else
#  define ESUP_LOG_AT(level, ...) ((void)0)
#  define ESUP_LOG_HEX(tag, data, len) ((void)0)
#endif

/** @brief Log at NORMAL level (white). */
#define ESUP_LOGN(...) ESUP_LOG_AT(ESUP_LOG_NORMAL, __VA_ARGS__)
/** @brief Log at INFO level (green). */
#define ESUP_LOGI(...) ESUP_LOG_AT(ESUP_LOG_INFO, __VA_ARGS__)
/** @brief Log at WARNING level (yellow). */
#define ESUP_LOGW(...) ESUP_LOG_AT(ESUP_LOG_WARNING, __VA_ARGS__)
/** @brief Log at ERROR level (red). */
#define ESUP_LOGE(...) ESUP_LOG_AT(ESUP_LOG_ERROR, __VA_ARGS__)

#endif /* ESUP_UTILS_H */
