/**
 * @file test_common.c
 * @brief CSP setup, file helpers, and tiny test-framework state.
 *
 * The extension-layer helpers (ping, csp_bufs, timeout, log, ram, shell,
 * kill) used to live here. They now live in ../client/ftpnew_client.c
 * which we link into test_client directly -- the tests call the same
 * ftpnew_* API the ground operator will use.
 */
#include "test_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include <csp/csp.h>
#include <csp/csp_iflist.h>
#include <csp/csp_rtable.h>
#include <csp/drivers/can_socketcan.h>

#include "ftpnew.h"   /* ftp_crc32_* */

/* ---- Test framework state -------------------------------------------- */

int  g_test_pass_count = 0;
int  g_test_fail_count = 0;
char g_test_last_msg[256];

/* ---- CSP setup ------------------------------------------------------- */

static csp_iface_t *g_can_iface = NULL;

int test_csp_init_node(uint8_t my_addr)
{
    csp_conf_t conf;
    csp_conf_get_defaults(&conf);

    conf.address           = my_addr;
    conf.hostname          = (my_addr == SERVER_ADDR) ? "ftpnew_test_server" : "ftpnew_test_client";
    conf.model             = "ftpnew_test";
    conf.revision          = "1";
    conf.conn_max          = TEST_CSP_CONN_MAX;
    conf.conn_queue_length = TEST_CSP_CONN_QUEUE;
    conf.fifo_length       = TEST_CSP_FIFO_LENGTH;
    conf.rdp_max_window    = TEST_CSP_RDP_MAX_WINDOW;
    conf.buffers           = TEST_CSP_BUFFERS;
    conf.buffer_data_size  = TEST_CSP_BUFFER_DATA_SIZE;

    int r = csp_init(&conf);
    if (r != CSP_ERR_NONE) {
        fprintf(stderr, "csp_init failed: %d\n", r);
        return r;
    }

    /* vcan0 in promiscuous mode -- both processes share the same wire and
     * we don't want the kernel filter dropping frames addressed to the
     * other endpoint. */
    r = csp_can_socketcan_open_and_add_interface(
            VCAN_DEVICE,
            CSP_IF_CAN_DEFAULT_NAME,
            0 /* bitrate: don't touch vcan */,
            true /* promisc */,
            &g_can_iface);
    if (r != CSP_ERR_NONE || g_can_iface == NULL) {
        fprintf(stderr, "csp_can_socketcan_open_and_add_interface(%s) failed: %d\n",
                VCAN_DEVICE, r);
        return (r != CSP_ERR_NONE) ? r : CSP_ERR_DRIVER;
    }

    /* Default route: everything via vcan0, no via address. */
    r = csp_rtable_set(0, 0, g_can_iface, CSP_NO_VIA_ADDRESS);
    if (r != CSP_ERR_NONE) {
        fprintf(stderr, "csp_rtable_set failed: %d\n", r);
        return r;
    }

    r = csp_route_start_task(16000, 0);
    if (r != CSP_ERR_NONE) {
        fprintf(stderr, "csp_route_start_task failed: %d\n", r);
        return r;
    }

    printf("[csp] node %u up on %s, buffers=%u, rdp_window<=%u\n",
           my_addr, VCAN_DEVICE, conf.buffers, conf.rdp_max_window);
    return CSP_ERR_NONE;
}

void test_csp_teardown(void)
{
    if (g_can_iface) {
        csp_can_socketcan_stop(g_can_iface);
        g_can_iface = NULL;
    }
    csp_free_resources();
}

/* ---- File helpers ---------------------------------------------------- */

int test_create_file(const char *path, size_t size, uint32_t seed)
{
    FILE *fp = fopen(path, "wb");
    if (!fp)
        return -1;

    /* xorshift32 -- deterministic, good enough for byte-compare. */
    uint32_t s = seed ? seed : 0xdeadbeefu;
    uint8_t buf[1024];
    while (size > 0) {
        size_t n = size > sizeof(buf) ? sizeof(buf) : size;
        for (size_t i = 0; i < n; ++i) {
            s ^= s << 13;
            s ^= s >> 17;
            s ^= s << 5;
            buf[i] = (uint8_t)(s & 0xff);
        }
        if (fwrite(buf, 1, n, fp) != n) {
            fclose(fp);
            return -1;
        }
        size -= n;
    }
    fclose(fp);
    return 0;
}

int test_files_equal(const char *a, const char *b)
{
    FILE *fa = fopen(a, "rb");
    FILE *fb = fopen(b, "rb");
    if (!fa || !fb) {
        if (fa) fclose(fa);
        if (fb) fclose(fb);
        return -1;
    }

    int rc = 0;
    uint8_t ba[4096], bb[4096];
    while (1) {
        size_t na = fread(ba, 1, sizeof(ba), fa);
        size_t nb = fread(bb, 1, sizeof(bb), fb);
        if (na != nb || memcmp(ba, bb, na) != 0) {
            rc = -1;
            break;
        }
        if (na == 0)
            break;
    }
    fclose(fa);
    fclose(fb);
    return rc;
}

int test_file_crc32(const char *path, uint32_t *crc)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
        return -1;

    uint32_t c = ftp_crc32_init();
    uint8_t buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        c = ftp_crc32_update(c, buf, n);

    *crc = ftp_crc32_finalize(c);
    fclose(fp);
    return 0;
}

int test_unlink(const char *path)
{
    if (unlink(path) != 0 && errno != ENOENT)
        return -1;
    return 0;
}

void test_scrub(const char *client_dir, const char *server_dir, const char *name)
{
    char p[512];

    if (client_dir) {
        snprintf(p, sizeof(p), "%s/%s",     client_dir, name); test_unlink(p);
        snprintf(p, sizeof(p), "%s/%s.tmp", client_dir, name); test_unlink(p);
        snprintf(p, sizeof(p), "%s/%s.map", client_dir, name); test_unlink(p);
    }
    if (server_dir) {
        snprintf(p, sizeof(p), "%s/%s",     server_dir, name); test_unlink(p);
        snprintf(p, sizeof(p), "%s/%s.tmp", server_dir, name); test_unlink(p);
        snprintf(p, sizeof(p), "%s/%s.map", server_dir, name); test_unlink(p);
    }
}
