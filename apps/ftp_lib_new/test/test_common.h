/**
 * @file test_common.h
 * @brief Shared test infrastructure for the ftpnew backend.
 *
 * Two-process design:
 *   - test_server: initializes CSP as node SERVER_ADDR, opens vcan0, hands
 *                  control to ftp_server_start() (in ftpnew.c) which loops
 *                  accepting connections on FTP_PORT and spawns one
 *                  ftp_handler_thread per connection.
 *   - test_client: initializes CSP as node CLIENT_ADDR, runs the test cases
 *                  via the ftpnew_* client API (in ../client/ftpnew_client.c).
 *
 * Both processes share vcan0. run_tests.sh brings the link up.
 */
#ifndef _FTP_TEST_COMMON_H_
#define _FTP_TEST_COMMON_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

/* ---- Topology --------------------------------------------------------- */

#define SERVER_ADDR     1
#define CLIENT_ADDR     2
#define FTP_PORT        9           /* matches ftpnew.c::ftp_server_start  */
#define VCAN_DEVICE     "vcan0"

/* ---- CSP buffer sizing (mirrors the flight config, see CLAUDE.md) ----- */

/* Match flight config exactly. The earlier bump was a band-aid for a
 * snapshot probe that was burning conn slots per test; the snapshot now
 * uses csp_buffer_remaining() locally, so the original limits are fine. */
#define TEST_CSP_BUFFERS            10
#define TEST_CSP_CONN_MAX           10
#define TEST_CSP_CONN_QUEUE         10
#define TEST_CSP_FIFO_LENGTH        25
#define TEST_CSP_RDP_MAX_WINDOW     20
#define TEST_CSP_BUFFER_DATA_SIZE   256

/* ---- Test framework --------------------------------------------------- */

extern int  g_test_pass_count;
extern int  g_test_fail_count;
extern char g_test_last_msg[256];

#define TEST_FAIL(fmt, ...) do {                                              \
        snprintf(g_test_last_msg, sizeof(g_test_last_msg),                    \
                 "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__);           \
        return -1;                                                            \
    } while (0)

#define TEST_ASSERT(cond, fmt, ...) do {                                      \
        if (!(cond))                                                          \
            TEST_FAIL("assert(" #cond ") -- " fmt, ##__VA_ARGS__);            \
    } while (0)

#define TEST_ASSERT_EQ(a, b, fmt, ...) do {                                   \
        long _aa = (long)(a), _bb = (long)(b);                                \
        if (_aa != _bb)                                                       \
            TEST_FAIL("expected %ld == %ld -- " fmt,                          \
                      _aa, _bb, ##__VA_ARGS__);                               \
    } while (0)

#define RUN_TEST(fn) do {                                                     \
        printf("\n[RUN ] %s\n", #fn);                                         \
        g_test_last_msg[0] = 0;                                               \
        int _r = (fn)();                                                      \
        if (_r == 0) {                                                        \
            printf("[ OK ] %s\n", #fn);                                       \
            g_test_pass_count++;                                              \
        } else {                                                              \
            printf("[FAIL] %s: %s\n", #fn, g_test_last_msg);                  \
            g_test_fail_count++;                                              \
        }                                                                     \
    } while (0)

/* ---- CSP node setup --------------------------------------------------- */

/**
 * Initialize CSP as @p my_addr, open vcan0 in promiscuous mode, install a
 * default route that sends every packet (mask 0) out through vcan0, and
 * start the router task. Returns CSP_ERR_NONE on success.
 */
int test_csp_init_node(uint8_t my_addr);

/**
 * Tear down CSP and stop the socketcan RX thread.
 */
void test_csp_teardown(void);

/* ---- File helpers ----------------------------------------------------- */

/**
 * Create a file of @p size bytes filled with a deterministic pseudo-random
 * pattern seeded from @p seed. Returns 0 on success.
 */
int test_create_file(const char *path, size_t size, uint32_t seed);

/**
 * Byte-compare two files. Returns 0 if identical, non-zero otherwise.
 */
int test_files_equal(const char *a, const char *b);

/**
 * Compute the IEEE 802.3 CRC32 of a file (whole file) using the same
 * ftp_crc32_* algorithm as the server / client. Returns 0 on success.
 */
int test_file_crc32(const char *path, uint32_t *crc);

/**
 * Delete file if it exists; ignore "not found".
 */
int test_unlink(const char *path);

/**
 * Best-effort scrub of any prior artifacts for @p name in both client and
 * server directories: the file itself, any .tmp, and any .map.
 */
void test_scrub(const char *client_dir, const char *server_dir, const char *name);

/* ---- Server entry point (defined in ../server/src/ftpnew.c) ----------- */

int ftp_server_start(void);

#endif /* _FTP_TEST_COMMON_H_ */
