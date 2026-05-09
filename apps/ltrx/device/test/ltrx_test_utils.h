/**
 * @file ltrx_test_utils.h
 * @author ryu@yonsei.ac.kr
 * @brief I like colorful logs.
 *        Don't forget to call log_init() first.
 * @version 1.0
 * @date 2026-03-12
 * 
 * ACL Yonsei, 2026
 */
#ifndef _LTRX_TEST_UTILS_H_
#define _LTRX_TEST_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Determines tag, indent and color. */
typedef enum {
    LOG_LEVEL_PASS,
    LOG_LEVEL_FAIL,
    LOG_LEVEL_SIM,
    LOG_LEVEL_SERIAL,
    LOG_LEVEL_INFO,
} log_level_t;

/* Max bytes to display in hexdump logs (single line). */
#define LOG_HEXDUMP_MAX_DISPLAY 16

void log_init(void);

/* All levels are verbose by default. Call to silence some. */
void log_set_verbose(log_level_t level, bool enabled);

void log_msg(log_level_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

void log_hexdump(log_level_t level, const void *data, size_t len);

void log_summary(int passed, int total);

#define LOG_PASS(...) \
    log_msg(LOG_LEVEL_PASS, __VA_ARGS__)
#define LOG_FAIL(...) \
    log_msg(LOG_LEVEL_FAIL, __VA_ARGS__)
#define LOG_SIM(...) \
    log_msg(LOG_LEVEL_SIM, __VA_ARGS__)
#define LOG_SERIAL(...) \
    log_msg(LOG_LEVEL_SERIAL, __VA_ARGS__)
#define LOG_INFO(...) \
    log_msg(LOG_LEVEL_INFO, __VA_ARGS__)

#endif