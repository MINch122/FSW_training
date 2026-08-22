/**
 * @file esup_utils.c
 * @brief Implementation of the compile-time-toggled ESUP debug logger.
 *
 * When ESUP_LOG is not defined (or zero) the runtime configuration entry points
 * still exist as no-ops so callers link unconditionally, but no logging code is
 * compiled in and the ESUP_LOG* macros expanded nowhere. When ESUP_LOG is set,
 * esup_log_write formats one coloured line per enabled level to stderr and, if
 * registered, to a second stream. Writes are serialized so lines from the
 * driver thread and the caller thread do not interleave.
 */
#include "esup_utils.h"

#if defined(ESUP_LOG) && (ESUP_LOG)

#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

/** @brief ANSI colour escape per level, indexed by esup_log_level_t. */
static const char* const level_color[] = {
    "\033[0m",  /* NORMAL  - default/white */
    "\033[32m", /* INFO    - green */
    "\033[33m", /* WARNING - yellow */
    "\033[31m", /* ERROR   - red */
    "\033[0m"   /* PACKET  - default/white */
};

/** @brief Human-readable tag per level, indexed by esup_log_level_t. */
static const char* const level_name[] = {"NORMAL", "INFO", "WARN", "ERROR",
                                          "PKT"};

#define LEVEL_COUNT 5
#define ANSI_RESET  "\033[0m"

static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
static bool level_enabled[LEVEL_COUNT] = {true, true, true, true, true};
static FILE* extra_stream = NULL;

void esup_log_enable(esup_log_level_t level, bool enabled)
{
    if ((unsigned)level >= LEVEL_COUNT)
        return;
    pthread_mutex_lock(&log_lock);
    level_enabled[level] = enabled;
    pthread_mutex_unlock(&log_lock);
}

void esup_log_set_file(FILE* stream)
{
    pthread_mutex_lock(&log_lock);
    extra_stream = stream;
    pthread_mutex_unlock(&log_lock);
}

/**
 * @brief Write one formatted, optionally coloured line to a single stream.
 *
 * @param stream Destination stream.
 * @param level  Message level (selects colour and tag).
 * @param func   Calling function name.
 * @param fmt    printf-style format.
 * @param ap     Argument list for @p fmt (consumed).
 */
static void write_line(FILE* stream, esup_log_level_t level, const char* func,
                       const char* fmt, va_list ap)
{
    bool color = isatty(fileno(stream)) != 0;

    if (color)
        fputs(level_color[level], stream);
    fprintf(stream, "[%s] %s: ", level_name[level], func);
    vfprintf(stream, fmt, ap);
    if (color)
        fputs(ANSI_RESET, stream);
    fputc('\n', stream);
}

void esup_log_write(esup_log_level_t level, const char* func, const char* fmt,
                    ...)
{
    if ((unsigned)level >= LEVEL_COUNT)
        return;

    pthread_mutex_lock(&log_lock);
    if (!level_enabled[level]) {
        pthread_mutex_unlock(&log_lock);
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    write_line(stderr, level, func, fmt, ap);
    va_end(ap);

    if (extra_stream != NULL) {
        va_start(ap, fmt);
        write_line(extra_stream, level, func, fmt, ap);
        va_end(ap);
    }
    pthread_mutex_unlock(&log_lock);
}

/**
 * @brief Write a run of bytes as spaced/grouped/rowed hex to one stream.
 *
 * Two hex digits per byte, a single space between bytes, a double space at every
 * ESUP_LOG_HEX_GROUP-byte boundary, and a newline at every ESUP_LOG_HEX_ROW-byte
 * boundary (no trailing separator).
 */
static void write_hex_run(FILE* stream, const uint8_t* p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        fprintf(stream, "%02X", (unsigned)p[i]);

        size_t done = i + 1u;

        if (done == n)
            break;
        if (done % ESUP_LOG_HEX_ROW == 0u)
            fputc('\n', stream);
        else if (done % ESUP_LOG_HEX_GROUP == 0u)
            fputs("  ", stream);
        else
            fputc(' ', stream);
    }
}

/**
 * @brief Write a full hex dump (header + body, middle elided) to one stream.
 */
static void write_hex(FILE* stream, esup_log_level_t level, const char* func,
                      const char* tag, const uint8_t* p, size_t len)
{
    bool color = isatty(fileno(stream)) != 0;

    if (color)
        fputs(level_color[level], stream);
    fprintf(stream, "[%s] %s: %s (%zu bytes)\n", level_name[level], func,
            (tag != NULL) ? tag : "", len);

    if (len <= ESUP_LOG_HEX_MAX) {
        write_hex_run(stream, p, len);
    } else {
        size_t half = ESUP_LOG_HEX_MAX / 2u;

        write_hex_run(stream, p, half);
        fputs("\n...\n", stream);
        write_hex_run(stream, p + (len - half), half);
    }
    if (color)
        fputs(ANSI_RESET, stream);
    fputc('\n', stream);
}

void esup_log_hex(esup_log_level_t level, const char* func, const char* tag,
                  const void* data, size_t len)
{
    if ((unsigned)level >= LEVEL_COUNT || data == NULL)
        return;

    pthread_mutex_lock(&log_lock);
    if (!level_enabled[level]) {
        pthread_mutex_unlock(&log_lock);
        return;
    }

    const uint8_t* p = data;

    write_hex(stderr, level, func, tag, p, len);
    if (extra_stream != NULL)
        write_hex(extra_stream, level, func, tag, p, len);
    pthread_mutex_unlock(&log_lock);
}

#else /* logging compiled out: keep the config entry points as no-ops */

void esup_log_enable(esup_log_level_t level, bool enabled)
{
    (void)level;
    (void)enabled;
}

void esup_log_set_file(FILE* stream)
{
    (void)stream;
}

void esup_log_write(esup_log_level_t level, const char* func, const char* fmt,
                    ...)
{
    (void)level;
    (void)func;
    (void)fmt;
}

void esup_log_hex(esup_log_level_t level, const char* func, const char* tag,
                  const void* data, size_t len)
{
    (void)level;
    (void)func;
    (void)tag;
    (void)data;
    (void)len;
}

#endif /* ESUP_LOG */
