/**
 * @file test_client.c
 * @brief Client-side test runner for the ftpnew backend.
 *
 * All upload / download tests go through the ftpnew_* wrappers (rather than
 * gs_ftp_upload directly) so we cover both the RDP and the CSP_O_CRC32-only
 * transport paths.
 *
 * All extension tests use the ftpnew_* extension client API in
 * ../client/ftpnew_client.c.
 */
#include "test_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <csp/csp.h>
#include <csp/csp_buffer.h>         /* csp_buffer_remaining for client-side leak check */
#include <gs/ftp/client.h>          /* gs_ftp_settings_t (type only) */
#include <gs/util/error.h>
#include "ftpnew_client.h"          /* ftpnew_upload_rdp / _udp, ftpnew_ping, ... */
#include "ftpnew.h"                 /* FTP_EXT_* constants, ftp_log_*_t, magic */

/* ---- Local filesystem layout ------------------------------------------ *
 *
 * Both processes run on the same host. To distinguish "what the server
 * sees" from "what the client sees" we keep them in separate working
 * directories; the client writes its local copies under CLIENT_DIR and
 * tells the server to put / get under SERVER_DIR.
 */
/* Short paths matter: GS_FTP_PATH_LENGTH is 50 (incl. NUL), so the
 * "<server_dir>/<filename>" the server sees must fit in 49 chars. With
 * a server_dir of "/tmp/ftpn_s" that leaves 38 chars for filenames. */
#ifndef CLIENT_DIR
#define CLIENT_DIR  "./ftpn_c"
#endif
#ifndef SERVER_DIR
#define SERVER_DIR  "./ftpn_s"
#endif

static char client_path_buf[256];
static char server_path_buf[256];

#define CPATH(name) (snprintf(client_path_buf, sizeof(client_path_buf), "%s/%s", CLIENT_DIR, (name)), client_path_buf)
#define SPATH(name) (snprintf(server_path_buf, sizeof(server_path_buf), "%s/%s", SERVER_DIR, (name)), server_path_buf)

static const char *remote_url(const char *name)
{
    static char buf[300];
    snprintf(buf, sizeof(buf), "file://" SERVER_DIR "/%s", name);
    return buf;
}

static const char *local_url(const char *name)
{
    static char buf[300];
    snprintf(buf, sizeof(buf), "file://" CLIENT_DIR "/%s", name);
    return buf;
}

static void default_settings(gs_ftp_settings_t *s, uint32_t chunk_size)
{
    memset(s, 0, sizeof(*s));
    s->host       = SERVER_ADDR;
    s->port       = FTP_PORT;
    s->mode       = GS_FTP_MODE_STANDARD;
    s->timeout    = 5000;
    s->chunk_size = chunk_size ? chunk_size : 185;
}

/* =========================================================================
 *                          BUFFER LEAK DETECTION
 * =========================================================================
 *
 * Per-test client-side probe via csp_buffer_remaining() -- process-local,
 * no wire traffic, no conn pool consumption. The server-side probe
 * (ftpnew_get_csp_buffers) opens a fresh CSP connection on every call,
 * which burns a slot on the server's conn pool for the full read-timeout
 * window after; doing that per test exhausted the pool after ~3 tests.
 * So we only sample the server side at baseline and at the final summary;
 * any sustained server-side leak shows up in baseline -> final delta.
 *
 * Sentinel (uint32_t)-1 means the server probe failed (server unreachable
 * or timed out). Only relevant to the baseline / final summary probes.
 */

static const uint32_t SNAP_FAIL = (uint32_t)-1;

static uint32_t client_free_now(void)
{
    return (uint32_t)csp_buffer_remaining();
}

static uint32_t server_free_now(void)
{
    gs_ftp_settings_t s;
    memset(&s, 0, sizeof(s));
    s.host    = SERVER_ADDR;
    s.port    = FTP_PORT;
    s.timeout = 1500;  /* don't hang on a dead server */
    uint32_t v = 0;
    if (ftpnew_get_csp_buffers(&s, &v, NULL) == GS_OK)
        return v;
    return SNAP_FAIL;
}

/* Override RUN_TEST so every test prints a client-side delta if nonzero.
 * Server-side delta is checked only at the end of the suite. The
 * trailing usleep lets in-flight CAN frames / CSP router activity drain
 * before the next test starts -- csp_send is non-blocking and the
 * router task is on its own thread, so a buffer "leaked" at snapshot
 * time is usually just one that hasn't been returned to the pool yet. */
#undef RUN_TEST
#define RUN_TEST(fn) do {                                                       \
        uint32_t _c0 = client_free_now();                                       \
        printf("\n[RUN ] %s\n", #fn);                                           \
        g_test_last_msg[0] = 0;                                                 \
        int _r = (fn)();                                                        \
        if (_r == 0) {                                                          \
            printf("[ OK ] %s\n", #fn);                                         \
            g_test_pass_count++;                                                \
        } else {                                                                \
            printf("[FAIL] %s: %s\n", #fn, g_test_last_msg);                    \
            g_test_fail_count++;                                                \
        }                                                                       \
        usleep(150 * 1000);                                                     \
        uint32_t _c1 = client_free_now();                                       \
        if (_c0 != _c1)                                                         \
            printf("       *** CLIENT BUFFER LEAK: %u -> %u (%+d)\n",           \
                   _c0, _c1, (int)_c1 - (int)_c0);                              \
    } while (0)

/* =========================================================================
 *                              UPLOAD TESTS
 * ========================================================================= */

/* Helper: run an upload-and-verify cycle. mode picks rdp vs udp. */
static int do_upload(const char *name, size_t size, uint32_t seed,
                     uint32_t chunk_size, bool use_rdp)
{
    test_scrub(CLIENT_DIR, SERVER_DIR, name);

    TEST_ASSERT(test_create_file(CPATH(name), size, seed) == 0, "create");

    gs_ftp_settings_t s;
    default_settings(&s, chunk_size);
    if (size > 4096)
        s.timeout = 30000;

    gs_error_t e = use_rdp
        ? ftpnew_upload_rdp(&s, local_url(name), remote_url(name), NULL, NULL)
        : ftpnew_upload_udp(&s, local_url(name), remote_url(name), NULL, NULL, 10);
    TEST_ASSERT_EQ(e, GS_OK, "upload (%s) returned %d", use_rdp ? "rdp" : "udp", e);
    TEST_ASSERT(test_files_equal(CPATH(name), SPATH(name)) == 0, "content mismatch");
    return 0;
}

static int test_upload_single_chunk_rdp(void)    { return do_upload("upload_single_rdp.bin",  100,   1, 185, true);  }
static int test_upload_single_chunk_udp(void)    { return do_upload("upload_single_udp.bin",  100,   2, 185, false); }
static int test_upload_multi_exact_rdp(void)     { return do_upload("upload_exact_rdp.bin",   1000,  3, 200, true);  }
static int test_upload_multi_exact_udp(void)     { return do_upload("upload_exact_udp.bin",   1000,  4, 200, false); }
static int test_upload_multi_partial_tail(void)  { return do_upload("upload_partial.bin",     1023,  5, 200, true);  }
static int test_upload_many_chunks_rdp(void)     { return do_upload("upload_many_rdp.bin",    65536, 6, 185, true);  }
static int test_upload_many_chunks_udp(void)     { return do_upload("upload_many_udp.bin",    65536, 7, 185, false); }

static int test_upload_empty_file(void)
{
    const char *name = "upload_empty.bin";
    test_scrub(CLIENT_DIR, SERVER_DIR, name);

    FILE *fp = fopen(CPATH(name), "wb");
    TEST_ASSERT(fp != NULL, "create");
    fclose(fp);

    gs_ftp_settings_t s;
    default_settings(&s, 185);

    gs_error_t e = ftpnew_upload_rdp(&s, local_url(name), remote_url(name), NULL, NULL);
    /* The backend's chunks==0 path is ill-defined; the only contract we
     * insist on is "does not crash and returns *some* result." */
    TEST_ASSERT(e == GS_OK || e == GS_ERROR_ARG ||
                e == GS_ERROR_NOT_SUPPORTED || e == GS_ERROR_IO,
                "upload of empty file returned %d", e);
    return 0;
}

/* Re-upload the same name on top of a previously-completed one. The map
 * file must have been removed at DONE, otherwise this fails with EXISTS. */
static int test_upload_repeat_same_name(void)
{
    const char *name = "upload_repeat.bin";
    test_scrub(CLIENT_DIR, SERVER_DIR, name);

    gs_ftp_settings_t s;
    default_settings(&s, 185);

    TEST_ASSERT(test_create_file(CPATH(name), 800, 11) == 0, "create 1");
    gs_error_t e = ftpnew_upload_rdp(&s, local_url(name), remote_url(name), NULL, NULL);
    TEST_ASSERT_EQ(e, GS_OK, "first upload %d", e);
    TEST_ASSERT(test_files_equal(CPATH(name), SPATH(name)) == 0, "first content");

    TEST_ASSERT(test_create_file(CPATH(name), 800, 22) == 0, "create 2");
    e = ftpnew_upload_rdp(&s, local_url(name), remote_url(name), NULL, NULL);
    TEST_ASSERT_EQ(e, GS_ERROR_EXIST, "second upload %d", e);
    return 0;
}

/* =========================================================================
 *                             DOWNLOAD TESTS
 * ========================================================================= */

static int do_download(const char *name, size_t size, uint32_t seed,
                       uint32_t chunk_size, bool use_rdp)
{
    test_scrub(CLIENT_DIR, SERVER_DIR, name);
    TEST_ASSERT(test_create_file(SPATH(name), size, seed) == 0, "create source");

    gs_ftp_settings_t s;
    default_settings(&s, chunk_size);
    if (size > 4096)
        s.timeout = 30000;

    gs_error_t e = use_rdp
        ? ftpnew_download_rdp(&s, local_url(name), remote_url(name), NULL, NULL)
        : ftpnew_download_udp(&s, local_url(name), remote_url(name), NULL, NULL);
    TEST_ASSERT_EQ(e, GS_OK, "download (%s) returned %d", use_rdp ? "rdp" : "udp", e);
    TEST_ASSERT(test_files_equal(CPATH(name), SPATH(name)) == 0, "content mismatch");
    return 0;
}

static int test_download_single_chunk_rdp(void)  { return do_download("download_single_rdp.bin", 120,   1, 185, true);  }
static int test_download_single_chunk_udp(void)  { return do_download("download_single_udp.bin", 120,   2, 185, false); }
static int test_download_multi_chunk_rdp(void)   { return do_download("download_multi_rdp.bin",  4096,  3, 185, true);  }
static int test_download_multi_chunk_udp(void)   { return do_download("download_multi_udp.bin",  4096,  4, 185, false); }
static int test_download_many_chunks(void)       { return do_download("download_many.bin",       65536, 5, 185, true);  }

static int test_download_nonexistent(void)
{
    const char *name = "definitely_not_there_xyz.bin";
    test_scrub(CLIENT_DIR, SERVER_DIR, name);

    gs_ftp_settings_t s;
    default_settings(&s, 185);

    gs_error_t e = ftpnew_download_rdp(&s, local_url(name), remote_url(name), NULL, NULL);
    /* Should fail. Backend maps fopen-fail to FTP_ERR_NOENT which
     * translates to GS_ERROR_NOT_FOUND on the client. */
    TEST_ASSERT(e != GS_OK, "expected failure, got OK");
    return 0;
}

/* =========================================================================
 *                                CRC TESTS
 * ========================================================================= */

static int test_crc_roundtrip(void)
{
    const char *up   = "crc_up.bin";
    const char *down = "crc_down.bin";
    test_scrub(CLIENT_DIR, SERVER_DIR, up);
    test_scrub(CLIENT_DIR, SERVER_DIR, down);

    TEST_ASSERT(test_create_file(CPATH(up), 2048, 8) == 0, "create");

    gs_ftp_settings_t s;
    default_settings(&s, 185);

    gs_error_t e = ftpnew_upload_rdp(&s, local_url(up), remote_url(up), NULL, NULL);
    TEST_ASSERT_EQ(e, GS_OK, "upload %d", e);

    e = ftpnew_download_rdp(&s, local_url(down), remote_url(up), NULL, NULL);
    TEST_ASSERT_EQ(e, GS_OK, "download %d", e);

    uint32_t c_orig = 0, c_round = 0;
    TEST_ASSERT(test_file_crc32(CPATH(up),   &c_orig)  == 0, "crc orig");
    TEST_ASSERT(test_file_crc32(CPATH(down), &c_round) == 0, "crc round");
    TEST_ASSERT_EQ(c_orig, c_round, "roundtrip CRC mismatch");
    return 0;
}

/* =========================================================================
 *                            EXTENSION TESTS
 * ========================================================================= */

static int test_ext_ping(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);
    TEST_ASSERT_EQ(ftpnew_ping(&s), GS_OK, "ping failed");
    return 0;
}

static int test_ext_csp_buffers(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    uint32_t remaining = 0, size = 0;
    TEST_ASSERT_EQ(ftpnew_get_csp_buffers(&s, &remaining, &size), GS_OK, "csp_bufs");
    printf("       buffers: remaining=%u size=%u\n", remaining, size);

    TEST_ASSERT(size >= TEST_CSP_BUFFER_DATA_SIZE,
                "buffer size %u smaller than data size %u",
                size, (unsigned)TEST_CSP_BUFFER_DATA_SIZE);
    TEST_ASSERT(remaining <= TEST_CSP_BUFFERS,
                "remaining %u > configured pool %u",
                remaining, (unsigned)TEST_CSP_BUFFERS);
    return 0;
}

static int test_ext_timeout_get(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    uint32_t timeout = 0, interpacket_delay = 0;
    TEST_ASSERT_EQ(ftpnew_get_timeout(&s, &timeout, &interpacket_delay), GS_OK, "get_timeout");
    printf("current timeout: %u ms, interpacket delay: %u ms\n", timeout, interpacket_delay);
    TEST_ASSERT(timeout > 0, "timeout should be nonzero");
    return 0;
}

static int test_ext_timeout_set_then_get(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    TEST_ASSERT_EQ(ftpnew_set_timeout(&s, 12345, 200), GS_OK, "set");
    uint32_t confirmed = 0;
    uint32_t packet_delay = 0;
    TEST_ASSERT_EQ(ftpnew_get_timeout(&s, &confirmed, &packet_delay), GS_OK, "get");
    TEST_ASSERT_EQ(confirmed, 12345, "set didn't echo new value for timeout");
    TEST_ASSERT_EQ(packet_delay, 200, "set didn't echo new value for interpacket delay");
    return 0;
}

/* After bouncing a few pings, verify the recent-tags reply includes them. */
static int test_ext_log_tags(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    /* Note: ftp_append_result is only called on ret != FTP_OK, so the
     * pings (which succeed) might not be logged. We exercise the API
     * regardless and just check it returns a well-formed reply. */
    TEST_ASSERT_EQ(ftpnew_clear_log(&s), GS_OK, "clear");
    TEST_ASSERT_EQ(ftpnew_ping(&s), GS_OK, "ping1");
    TEST_ASSERT_EQ(ftpnew_ping(&s), GS_OK, "ping2");

    /* Force an error that *does* get logged: download a nonexistent file. */
    TEST_ASSERT_EQ(ftpnew_download_rdp(&s, local_url("nope_log.bin"),
                                       remote_url("nope_log.bin"), NULL, NULL)
                   != GS_OK, true, "nope download should fail");

    ftp_log_tag_t tags[8];
    int n = 0;
    TEST_ASSERT_EQ(ftpnew_get_log_tags(&s, 8, tags, &n), GS_OK, "tags");
    printf("       received %d log tags\n", n);
    for (int i = 0; i < n; i++) {
        printf("         tag[%d]: index=%u csp_time=%u type=%u ret=%u\n",
               i,
               tags[i].index,
               tags[i].csp_time,
               tags[i].type,
               tags[i].ret);
    }
    TEST_ASSERT(n >= 1, "expected at least one tag from the failed download");
    return 0;
}

/* Read back a single entry with its message text. */
static int test_ext_log_entry(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    TEST_ASSERT_EQ(ftpnew_clear_log(&s), GS_OK, "clear");

    /* Trigger an error that produces a log entry with a known message. */
    TEST_ASSERT_EQ(ftpnew_download_rdp(&s, local_url("nope_entry.bin"),
                                       remote_url("nope_entry.bin"), NULL, NULL)
                   != GS_OK, true, "nope download should fail");

    /* The most recent entry (index 0) should be the failed download. */
    ftp_log_entry_t entry;
    memset(&entry, 0, sizeof(entry));
    gs_error_t e = ftpnew_get_log_entry(&s, 0, &entry);
    TEST_ASSERT_EQ(e, GS_OK, "get_log_entry %d", e);

    printf("       entry[0]:\n");
    printf("         index    = %u\n", (unsigned)entry.tag.index);
    printf("         csp_time = %u\n", (unsigned)entry.tag.csp_time);
    printf("         type     = %u\n", (unsigned)entry.tag.type);
    printf("         ret      = %d\n", (int)entry.tag.ret);
    printf("         message  = '%s'\n", entry.message);

    /* Message should be non-empty (the handler emitted a debug_error). */
    TEST_ASSERT(entry.message[0] != 0,
                "log message field is empty -- ftp_append_result didn't pick up the debug msg");
    return 0;
}

/* =========================================================================
 *                            RAM ext (shape only)
 * =========================================================================
 *
 * We don't actually verify the RAM bytes -- doing so safely from the
 * ground side requires the satellite to advertise a writable scratch
 * region. What we DO verify is that the protocol round-trips: request
 * succeeds, reply has the right shape.
 *
 * If you ever wire a "scratch buffer" mechanism on the server, you can
 * replace the address here with that buffer's address and uncomment the
 * byte-compare assertions.
 */

/* The server's ram_read / ram_write handlers do `memcpy` to / from a raw
 * address supplied by the client. From the ground side we have no way to
 * name a satellite-side address that is safe to access; any test that
 * actually sends a valid request triggers a memcpy to an arbitrary host
 * address in the server process, which SEGVs the server and kills every
 * downstream test with -5 (IO).
 *
 * Exercise only the client-side path so the server is never touched. We
 * pass size > FTP_EXT_RAM_MAX_BYTES; ftpnew_ram_{read,write} reject it
 * locally with GS_ERROR_ARG before the request leaves the host. The wire
 * path can be re-enabled once the server exposes a known-safe scratch
 * region. */

static int test_ext_ram_write(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    uint8_t buf[1];
    gs_error_t e = ftpnew_ram_write(&s, /*offset*/ 0, /*size*/ 1024, buf);
    TEST_ASSERT_EQ(e, GS_ERROR_ARG,
                   "client-side validation should reject size>256, got %d", e);
    return 0;
}

static int test_ext_ram_read(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    uint8_t buf[1];
    gs_error_t e = ftpnew_ram_read(&s, /*offset*/ 0, /*size*/ 1024, buf);
    TEST_ASSERT_EQ(e, GS_ERROR_ARG,
                   "client-side validation should reject size>256, got %d", e);
    return 0;
}

/* =========================================================================
 *                          Shell command (fork+exec)
 * =========================================================================
 *
 * The server's shell handler forks /bin/sh -c <cmd> and reports the exit
 * status. We test three cases: trivially-OK, intentional failure, and
 * not-found.
 */

static int test_ext_shell_true(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    int sysret = -123;
    gs_error_t e = ftpnew_shell_cmd(&s, "true", &sysret);
    TEST_ASSERT_EQ(e, GS_OK, "shell true returned %d", e);
    TEST_ASSERT_EQ(sysret, 0, "true should exit 0, got %d", sysret);
    return 0;
}

static int test_ext_shell_false(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    int sysret = -123;
    gs_error_t e = ftpnew_shell_cmd(&s, "false", &sysret);
    TEST_ASSERT_EQ(e, GS_OK, "shell false returned %d", e);
    TEST_ASSERT_EQ(sysret, 1, "false should exit 1, got %d", sysret);
    return 0;
}

static int test_ext_shell_not_found(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 0);

    int sysret = -123;
    gs_error_t e = ftpnew_shell_cmd(&s, "/nonexistent/binary_xyz", &sysret);
    TEST_ASSERT_EQ(e, GS_OK, "shell exec returned %d (the call itself is OK)", e);
    /* Convention used by the server: execl-fail -> _exit(127). */
    TEST_ASSERT_EQ(sysret, 127, "expected sysret=127 for command-not-found, got %d", sysret);
    return 0;
}

/* =========================================================================
 *                            ROBUSTNESS / STRESS
 * ========================================================================= */

static int test_sequence_three_uploads(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 185);

    for (int i = 0; i < 3; ++i) {
        char name[64];
        snprintf(name, sizeof(name), "seq_%d.bin", i);
        test_scrub(CLIENT_DIR, SERVER_DIR, name);

        TEST_ASSERT(test_create_file(CPATH(name), 500 + i * 137, 100 + i) == 0,
                    "create iter %d", i);

        gs_error_t e = ftpnew_upload_rdp(&s, local_url(name), remote_url(name), NULL, NULL);
        TEST_ASSERT_EQ(e, GS_OK, "upload iter %d returned %d", i, e);
        TEST_ASSERT(test_files_equal(CPATH(name), SPATH(name)) == 0,
                    "content mismatch on iter %d", i);
    }
    return 0;
}

/* After a barrage of transfers, the satellite-side buffer pool should
 * settle back to its idle level. This is the diagnostic we set up for the
 * RDP/buffer-leak investigation. */
static int test_buffer_pool_stable(void)
{
    gs_ftp_settings_t s;
    default_settings(&s, 185);

    uint32_t before = 0, after = 0;
    TEST_ASSERT_EQ(ftpnew_get_csp_buffers(&s, &before, NULL), GS_OK, "query before");

    for (int i = 0; i < 3; ++i) {
        char name[64];
        snprintf(name, sizeof(name), "leakcheck_%d.bin", i);
        test_scrub(CLIENT_DIR, SERVER_DIR, name);
        TEST_ASSERT(test_create_file(CPATH(name), 2048, 50 + i) == 0, "create");
        gs_error_t e = ftpnew_upload_rdp(&s, local_url(name), remote_url(name), NULL, NULL);
        TEST_ASSERT_EQ(e, GS_OK, "upload %d", i);
    }
    /* Let the connection teardown propagate. */
    sleep(1);

    TEST_ASSERT_EQ(ftpnew_get_csp_buffers(&s, &after, NULL), GS_OK, "query after");
    printf("       buffers remaining: before=%u after=%u\n", before, after);
    TEST_ASSERT(after + 1 >= before,
                "buffer pool shrank: %u -> %u", before, after);
    return 0;
}

/* =========================================================================
 *                              MAIN
 * ========================================================================= */

static int ensure_dirs(void)
{
    struct stat st;
    if (stat(CLIENT_DIR, &st) != 0 && mkdir(CLIENT_DIR, 0755) != 0) {
        perror("mkdir client dir");
        return -1;
    }
    if (stat(SERVER_DIR, &st) != 0 && mkdir(SERVER_DIR, 0755) != 0) {
        perror("mkdir server dir");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    if (ensure_dirs() != 0)
        return 1;

    if (test_csp_init_node(CLIENT_ADDR) != 0) {
        fprintf(stderr, "[client] CSP init failed\n");
        return 1;
    }

    typedef struct {
        unsigned window_size;
        unsigned conn_timeout_ms;
        unsigned packet_timeout_ms;
        unsigned delayed_acks;
        unsigned ack_timeout;
        unsigned ack_delay_count;
    } rdp_opt_snapshot_t;

    rdp_opt_snapshot_t rdp_opts;

    csp_rdp_get_opt(&rdp_opts.window_size, &rdp_opts.conn_timeout_ms, &rdp_opts.packet_timeout_ms,
                    &rdp_opts.delayed_acks, &rdp_opts.ack_timeout, &rdp_opts.ack_delay_count);
    csp_rdp_set_opt(rdp_opts.window_size, 10, rdp_opts.packet_timeout_ms,
                    rdp_opts.delayed_acks, rdp_opts.ack_timeout, rdp_opts.ack_delay_count);

    sleep(1);  /* let the server come up */

    printf("\n=========== ftpnew test suite ===========\n");

    uint32_t baseline_client = client_free_now();
    uint32_t baseline_server = server_free_now();
    printf("[baseline] client_free=%u  server_free=%s%u\n",
           baseline_client,
           baseline_server == SNAP_FAIL ? "(probe failed) " : "",
           baseline_server);

    /* ---- Upload ---- */
    RUN_TEST(test_upload_single_chunk_rdp);
    RUN_TEST(test_upload_single_chunk_udp);
    RUN_TEST(test_upload_multi_exact_rdp);
    RUN_TEST(test_upload_multi_exact_udp);
    RUN_TEST(test_upload_multi_partial_tail);
    RUN_TEST(test_upload_many_chunks_rdp);
    RUN_TEST(test_upload_many_chunks_udp);
    RUN_TEST(test_upload_empty_file);
    RUN_TEST(test_upload_repeat_same_name);

    /* ---- Download ---- */
    RUN_TEST(test_download_single_chunk_rdp);
    RUN_TEST(test_download_single_chunk_udp);
    RUN_TEST(test_download_multi_chunk_rdp);
    RUN_TEST(test_download_multi_chunk_udp);
    RUN_TEST(test_download_many_chunks);
    RUN_TEST(test_download_nonexistent);

    /* ---- CRC ---- */
    RUN_TEST(test_crc_roundtrip);

    /* ---- Extensions (ping / bufs / timeout / log) ---- */
    RUN_TEST(test_ext_ping);
    RUN_TEST(test_ext_csp_buffers);
    RUN_TEST(test_ext_timeout_get);
    RUN_TEST(test_ext_timeout_set_then_get);
    RUN_TEST(test_ext_log_tags);
    RUN_TEST(test_ext_log_entry);

    /* ---- RAM ext (shape-only, see notes in the test bodies) ---- */
    RUN_TEST(test_ext_ram_write);
    RUN_TEST(test_ext_ram_read);

    /* ---- Shell exec ---- */
    RUN_TEST(test_ext_shell_true);
    RUN_TEST(test_ext_shell_false);
    RUN_TEST(test_ext_shell_not_found);

    /* ---- Robustness ---- */
    RUN_TEST(test_sequence_three_uploads);
    RUN_TEST(test_buffer_pool_stable);

    uint32_t final_client = client_free_now();
    uint32_t final_server = server_free_now();

    printf("\n=========================================\n");
    printf("Result: %d passed, %d failed\n",
           g_test_pass_count, g_test_fail_count);
    printf("\n[buffers]  client:  baseline=%u  final=%u  delta=%+d\n",
           baseline_client, final_client,
           (int)final_client - (int)baseline_client);
    if (baseline_server != SNAP_FAIL && final_server != SNAP_FAIL)
        printf("[buffers]  server:  baseline=%u  final=%u  delta=%+d\n",
               baseline_server, final_server,
               (int)final_server - (int)baseline_server);
    else
        printf("[buffers]  server:  probe failed (baseline=%u final=%u)\n",
               baseline_server, final_server);
    printf("=========================================\n");

    test_csp_teardown();
    return g_test_fail_count ? 1 : 0;
}
