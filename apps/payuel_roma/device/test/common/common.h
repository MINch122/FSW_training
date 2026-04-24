#ifndef _S5LAB_TEST_COMMON_H_
#define _S5LAB_TEST_COMMON_H_

#include <stdint.h>

#define HOST_NODE_ADDRESS 3

#define DEVICE_NODE_ADDRESS 10

#define HOST_BUFFER_SIZE 256

#define DEVICE_BUFFER_SIZE 256

#define VCAN_INTERFACE "vcan0"

int init_csp(uint8_t node, uint16_t buffer_size, uint16_t nbuffers);

int init_can_iface(const char* bus, uint8_t dst_node);

static int test_failed __attribute__((unused)) = 0;
static int test_passed __attribute__((unused)) = 0;

void LOG_PASS(const char *fmt, ...);
void LOG_FAIL(const char *fmt, ...);
void log_summary(int passed, int total);

#define SUMMARY() \
    do { \
        log_summary(test_passed, test_passed + test_failed); \
        exit(test_failed > 0 ? 1 : 0); \
    } while (0)

#define TEST_EQ(expr, expected) \
    do { \
        int64_t result = (expr); \
        if (result == (expected)) { \
            LOG_PASS(#expr); \
            test_passed++; \
        } else { \
            LOG_FAIL(#expr ": expected %" PRId64 ", got %" PRId64, (int64_t)(expected), (int64_t)result); \
            test_failed++; \
        } \
    } while (0)

#define TEST_NEQ(expr, not_expected) \
    do { \
        int64_t result = (expr); \
        if (result != (int64_t)(not_expected)) { \
            LOG_PASS(#expr); \
            test_passed++; \
        } else { \
            LOG_FAIL(#expr ": not expected %" PRId64 ", but indeed got %" PRId64, (int64_t)(not_expected), (int64_t)result); \
            test_failed++; \
        } \
    } while (0)

#endif
