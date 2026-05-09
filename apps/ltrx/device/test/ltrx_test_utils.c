/**
 * @file ltrx_test_utils.c
 * @author ryu@yonsei.ac.kr
 * @brief I like colorful logs.
 *        Don't forget to call log_init() first.
 * @version 1.0
 * @date 2026-03-12
 * 
 * ACL Yonsei, 2026
 */
#include "ltrx_test_utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

static bool use_color = false;

#define CLR_RESET  "\033[0m"
#define CLR_RED    "\033[31m"
#define CLR_GREEN  "\033[32m"
#define CLR_YELLOW "\033[33m"
#define CLR_CYAN   "\033[36m"
#define CLR_BWHITE "\033[97m"
#define CLR_DIM    "\033[2m"
#define CLR_MAGENTA "\033[35m"
#define CLR_BOLD   "\033[1m"

typedef struct {
    const char *color;
    const char *tag;
    int indentLevel;
} log_style_t;

static bool verbose_levels[5] = { true, true, true, true, true };

static const log_style_t styles[] = {
    [LOG_LEVEL_PASS]   = { CLR_GREEN,          " PASS ",   0 },
    [LOG_LEVEL_FAIL]   = { CLR_RED CLR_BOLD,   " FAIL ",   0 },
    [LOG_LEVEL_SIM]    = { CLR_CYAN,           "[SIM]",    1 },
    [LOG_LEVEL_SERIAL] = { CLR_MAGENTA,            "[SERIAL]", 2 },
    [LOG_LEVEL_INFO]   = { CLR_YELLOW,         "[INFO]",   0 },
};

void log_init(void)
{
    use_color = isatty(STDOUT_FILENO);
}

void log_set_verbose(log_level_t level, bool enabled)
{
    if (level < 0 || level >= sizeof(verbose_levels) / sizeof(verbose_levels[0]))
        return;
    verbose_levels[level] = enabled;
}

void log_msg(log_level_t level, const char *fmt, ...)
{
    if (!verbose_levels[level])
        return;

    const log_style_t *s = &styles[level];

    for (int i = 0; i < s->indentLevel; i++)
        printf("  ");

    if (use_color)
        printf("%s%s%s ", s->color, s->tag, CLR_RESET);
    else
        printf("%s ", s->tag);


    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    putchar('\n');
}

void log_hexdump(log_level_t level, const void *data, size_t len)
{
    if (!verbose_levels[level])
        return;

    const uint8_t *p = data;

    const log_style_t *s = &styles[level];

    for (int i = 0; i < s->indentLevel; i++)
        printf("  ");

    if (use_color)
        printf("%s%s%s ", s->color, s->tag, CLR_RESET);
    else
        printf("%s ", s->tag);

    for (size_t i = 0; i < len && i < LOG_HEXDUMP_MAX_DISPLAY; i++)
        printf("%02X ", p[i]);

    if (len > LOG_HEXDUMP_MAX_DISPLAY)
        printf("... (%zu bytes total)", len);

    putchar('\n');
}

void log_summary(int passed, int total)
{
    bool all_passed = (passed == total);
    const char *color = all_passed ? CLR_GREEN CLR_BOLD : CLR_RED CLR_BOLD;

    if (use_color)
        printf("\n%s%d/%d tests passed%s\n", color, passed, total, CLR_RESET);
    else
        printf("\n%d/%d tests passed\n", passed, total);
}
