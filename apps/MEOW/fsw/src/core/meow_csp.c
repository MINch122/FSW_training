/**
 * @file meow_csp.c
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: CSP (CubeSat Space Protocol) wrapper utilities.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#include "meow_csp.h"

#include <csp/csp_iflist.h>
#include <csp/csp_rtable.h>
#include <gs/ftp/client.h>

#include "osapi-module.h"
#include "osapi-error.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>

#define SERVER_BACKLOG     5
#define ROUTER_STACK_SIZE  1024
#define ROUTER_PRIORITY    0
#define SERVER_ACCEPT_TMO  1000  /* ms; sets the granularity of stop responsiveness */
#define FORWARD_TMO_DEFAULT 1000 /* ms; used when reroute entry has timeout_ms == 0 */

/* -------------------------------------------------------------------------
 * Module state
 * ---------------------------------------------------------------------- */

static csp_socket_t*    server_sock     = NULL;
static pthread_t        server_thread;
static volatile int     server_running  = 0;
static volatile uint32_t server_read_timeout = 500;

static meow_csp_reroute_entry_t reroute_table[MEOW_CSP_REROUTE_MAX];

typedef struct {
    uint8_t                  port;
    meow_csp_port_handler    fn;
    void*                    ctx;
    osal_id_t                module_id;  /* valid only when has_module == 1 */
    int                      has_module; /* 1 → unload module_id on handler_clear */
    int                      active;
} port_handler_t;

static port_handler_t  handler_table[MEOW_CSP_HANDLER_MAX];
static pthread_mutex_t handler_mtx = PTHREAD_MUTEX_INITIALIZER;

static int last_csp_err    = 0;
static int last_ftp_err    = 0;
static int last_module_err = 0;

static csp_conf_t default_conf = {
    .address          = 1,
    .hostname         = "MEOW",
    .model            = "cFS",
    .revision         = "",
    .buffers          = 10,
    .buffer_data_size = 256,
};

/* -------------------------------------------------------------------------
 * Raw error accessors
 * ---------------------------------------------------------------------- */

int meow_csp_last_err(void)
{
    return last_csp_err;
}

int meow_csp_last_ftp_err(void)
{
    return last_ftp_err;
}

int meow_csp_last_module_err(void)
{
    return last_module_err;
}

void meow_csp_set_read_timeout(uint32_t timeout_ms)
{
    server_read_timeout = timeout_ms;
}

uint32_t meow_csp_get_read_timeout(void)
{
    return server_read_timeout;
}

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

int meow_csp_init(const csp_conf_t* conf)
{
    const csp_conf_t* csp_conf = conf ? conf : &default_conf;

    int ret = csp_init(csp_conf);
    if (ret != CSP_ERR_NONE) {
        last_csp_err = ret;
        return MEOW_CSP_ERR_INIT_CSP;
    }

    ret = csp_route_start_task(ROUTER_STACK_SIZE, ROUTER_PRIORITY);
    if (ret != CSP_ERR_NONE) {
        last_csp_err = ret;
        return MEOW_CSP_ERR_INIT_TASK;
    }

    return MEOW_CSP_OK;
}

/* -------------------------------------------------------------------------
 * Server thread
 * ---------------------------------------------------------------------- */

void meow_csp_dispatch(csp_conn_t* conn, csp_packet_t* packet)
{
    uint8_t port = (uint8_t)csp_conn_dport(conn);

    /* 1 — custom handler table */
    pthread_mutex_lock(&handler_mtx);
    meow_csp_port_handler fn = NULL;
    void* ctx = NULL;
    for (int i = 0; i < MEOW_CSP_HANDLER_MAX; i++) {
        if (handler_table[i].active && handler_table[i].port == port) {
            fn  = handler_table[i].fn;
            ctx = handler_table[i].ctx;
            break;
        }
    }
    pthread_mutex_unlock(&handler_mtx);

    if (fn) {
        fn(conn, packet, ctx);
        return;
    }

    csp_service_handler(conn, packet);
}

static void* server_thread_func(void* arg)
{
    (void)arg;

    /**
     * Breaks out if server_running is set to 0 by meow_csp_server_stop().
     */
    while (server_running) {
        csp_conn_t* conn = csp_accept(server_sock, SERVER_ACCEPT_TMO);
        if (!conn)
            continue;

        csp_packet_t* packet;
        while ((packet = csp_read(conn, server_read_timeout)) != NULL)
            meow_csp_dispatch(conn, packet);

        csp_close(conn);
    }

    return NULL;
}

int meow_csp_server_start(void)
{
    if (server_running)
        return MEOW_CSP_ERR_THREAD;

    server_sock = csp_socket(CSP_SO_NONE);
    if (!server_sock)
        return MEOW_CSP_ERR_SOCKET;

    if (csp_bind(server_sock, CSP_ANY) != CSP_ERR_NONE) {
        csp_close(server_sock);
        server_sock = NULL;
        return MEOW_CSP_ERR_SOCKET;
    }

    if (csp_listen(server_sock, SERVER_BACKLOG) != CSP_ERR_NONE) {
        csp_close(server_sock);
        server_sock = NULL;
        return MEOW_CSP_ERR_SOCKET;
    }

    server_running = 1;
    if (pthread_create(&server_thread, NULL, server_thread_func, NULL) != 0) {
        server_running = 0;
        csp_close(server_sock);
        server_sock = NULL;
        return MEOW_CSP_ERR_THREAD;
    }

    return MEOW_CSP_OK;
}

int meow_csp_server_stop(void)
{
    if (!server_running)
        return MEOW_CSP_OK;

    server_running = 0;
    if (pthread_join(server_thread, NULL) != 0)
        return MEOW_CSP_ERR_THREAD;

    return MEOW_CSP_OK;
}

static void reroute_fn(csp_conn_t* conn, csp_packet_t* packet, void* ctx)
{
    const meow_csp_reroute_entry_t* rule = (const meow_csp_reroute_entry_t*)ctx;

    if (rule->src_node != 0 && (uint8_t)csp_conn_src(conn) != rule->src_node) {
        csp_service_handler(conn, packet);
        return;
    }

    uint32_t tmo = rule->timeout_ms ? rule->timeout_ms : FORWARD_TMO_DEFAULT;
    int ret = csp_sendto(CSP_PRIO_NORM,
                         rule->fwd_dst, rule->fwd_dst_port,
                         rule->fwd_src_port, CSP_SO_NONE, packet, tmo);
    if (ret != CSP_ERR_NONE) {
        csp_buffer_free(packet);
    }
}

int meow_csp_handler_set(uint8_t port, meow_csp_port_handler fn, void* ctx)
{
    if (!fn)
        return MEOW_CSP_ERR_NULL;

    pthread_mutex_lock(&handler_mtx);

    int free_slot = -1;
    for (int i = 0; i < MEOW_CSP_HANDLER_MAX; i++) {
        if (handler_table[i].active && handler_table[i].port == port) {
            handler_table[i].fn  = fn;
            handler_table[i].ctx = ctx;
            pthread_mutex_unlock(&handler_mtx);
            return MEOW_CSP_OK;
        }
        if (!handler_table[i].active && free_slot < 0)
            free_slot = i;
    }

    if (free_slot < 0) {
        pthread_mutex_unlock(&handler_mtx);
        return MEOW_CSP_ERR_BOUNDS;
    }

    handler_table[free_slot].port       = port;
    handler_table[free_slot].fn         = fn;
    handler_table[free_slot].ctx        = ctx;
    handler_table[free_slot].has_module = 0;
    handler_table[free_slot].active     = 1;
    pthread_mutex_unlock(&handler_mtx);
    return MEOW_CSP_OK;
}

int meow_csp_handler_load(uint8_t port, const char* path, const char* symbol)
{
    if (!path || !symbol)
        return MEOW_CSP_ERR_NULL;

    osal_id_t module_id;
    int32 ret = OS_ModuleLoad(&module_id, symbol, path, OS_MODULE_FLAG_LOCAL_SYMBOLS);
    if (ret != OS_SUCCESS) {
        last_module_err = ret;
        return MEOW_CSP_ERR_MODULE;
    }

    cpuaddr addr;
    ret = OS_ModuleSymbolLookup(module_id, &addr, symbol);
    if (ret != OS_SUCCESS) {
        last_module_err = ret;
        OS_ModuleUnload(module_id);
        return MEOW_CSP_ERR_MODULE;
    }

    meow_csp_port_handler fn = (meow_csp_port_handler)(uintptr_t)addr;

    pthread_mutex_lock(&handler_mtx);

    int slot = -1;
    int free_slot = -1;
    for (int i = 0; i < MEOW_CSP_HANDLER_MAX; i++) {
        if (handler_table[i].active && handler_table[i].port == port) {
            slot = i;
            break;
        }
        if (!handler_table[i].active && free_slot < 0)
            free_slot = i;
    }

    if (slot < 0) {
        if (free_slot < 0) {
            pthread_mutex_unlock(&handler_mtx);
            OS_ModuleUnload(module_id);
            return MEOW_CSP_ERR_BOUNDS;
        }
        slot = free_slot;
    } else if (handler_table[slot].has_module) {
        OS_ModuleUnload(handler_table[slot].module_id);
    }

    handler_table[slot].port       = port;
    handler_table[slot].fn         = fn;
    handler_table[slot].ctx        = NULL;
    handler_table[slot].module_id  = module_id;
    handler_table[slot].has_module = 1;
    handler_table[slot].active     = 1;
    pthread_mutex_unlock(&handler_mtx);
    return MEOW_CSP_OK;
}

int meow_csp_handler_clear(uint8_t port)
{
    pthread_mutex_lock(&handler_mtx);
    for (int i = 0; i < MEOW_CSP_HANDLER_MAX; i++) {
        if (handler_table[i].active && handler_table[i].port == port) {
            osal_id_t mid        = handler_table[i].module_id;
            int       has_module = handler_table[i].has_module;
            memset(&handler_table[i], 0, sizeof(handler_table[i]));
            pthread_mutex_unlock(&handler_mtx);
            if (has_module)
                OS_ModuleUnload(mid);
            return MEOW_CSP_OK;
        }
    }
    pthread_mutex_unlock(&handler_mtx);
    return MEOW_CSP_OK;
}

/* -------------------------------------------------------------------------
 * Send
 * ---------------------------------------------------------------------- */

int meow_csp_send(uint8_t dst, uint8_t dst_port, uint8_t src_port,
                  uint8_t prio, const void* data, uint16_t len,
                  uint32_t timeout_ms)
{
    if (!data || len == 0)
        return MEOW_CSP_ERR_SEND;

    csp_packet_t* packet = csp_buffer_get(len);
    if (!packet)
        return MEOW_CSP_ERR_NOMEM;

    memcpy(packet->data, data, len);
    packet->length = len;

    int ret = csp_sendto(prio, dst, dst_port, src_port, CSP_SO_NONE, packet, timeout_ms);
    if (ret != CSP_ERR_NONE) {
        last_csp_err = ret;
        csp_buffer_free(packet);
        return MEOW_CSP_ERR_SEND;
    }

    return MEOW_CSP_OK;
}

/* -------------------------------------------------------------------------
 * FTP
 * ---------------------------------------------------------------------- */

int meow_csp_ftp_upload(uint8_t host, uint8_t port,
                        const char* local_path, const char* remote_path,
                        uint32_t timeout_ms, uint32_t chunk_size)
{
    if (!local_path || !remote_path)
        return MEOW_CSP_ERR_NULL;

    gs_ftp_settings_t s;
    gs_ftp_default_settings(&s);
    s.host = host;
    if (port)
        s.port = port;
    if (timeout_ms)
        s.timeout = timeout_ms;
    if (chunk_size)
        s.chunk_size = chunk_size;

    gs_error_t err = gs_ftp_upload(&s, local_path, remote_path, NULL, NULL);
    if (err != GS_OK) {
        last_ftp_err = err;
        return MEOW_CSP_ERR_FTP;
    }
    return MEOW_CSP_OK;
}

int meow_csp_ftp_download(uint8_t host, uint8_t port,
                          const char* local_path, const char* remote_path,
                          uint32_t timeout_ms, uint32_t chunk_size)
{
    if (!local_path || !remote_path)
        return MEOW_CSP_ERR_NULL;

    gs_ftp_settings_t s;
    gs_ftp_default_settings(&s);
    s.host = host;
    if (port)
        s.port = port;
    if (timeout_ms)
        s.timeout = timeout_ms;
    if (chunk_size)
        s.chunk_size = chunk_size;

    gs_error_t err = gs_ftp_download(&s, local_path, remote_path, NULL, NULL);
    if (err != GS_OK) {
        last_ftp_err = err;
        return MEOW_CSP_ERR_FTP;
    }
    return MEOW_CSP_OK;
}

/* -------------------------------------------------------------------------
 * Interface stats / routing
 * ---------------------------------------------------------------------- */

int meow_csp_ifstats(const char* iface_name, meow_csp_ifstats_t* stats)
{
    if (!iface_name || !stats)
        return MEOW_CSP_ERR_NULL;

    csp_iface_t* iface = csp_iflist_get_by_name(iface_name);
    if (!iface)
        return MEOW_CSP_ERR_IFACE;

    stats->tx       = iface->tx;
    stats->rx       = iface->rx;
    stats->tx_error = iface->tx_error;
    stats->rx_error = iface->rx_error;
    stats->drop     = iface->drop;

    return MEOW_CSP_OK;
}

int meow_csp_route_set(uint8_t dst, uint8_t mask,
                       const char* iface_name, uint8_t via)
{
    if (!iface_name)
        return MEOW_CSP_ERR_NULL;

    csp_iface_t* iface = csp_iflist_get_by_name(iface_name);
    if (!iface)
        return MEOW_CSP_ERR_IFACE;

    int ret = csp_rtable_set(dst, mask, iface, via);
    if (ret != CSP_ERR_NONE) {
        last_csp_err = ret;
        return MEOW_CSP_ERR_ROUTE;
    }

    return MEOW_CSP_OK;
}

/* -------------------------------------------------------------------------
 * Reroute table
 * ---------------------------------------------------------------------- */

int meow_csp_reroute_set(uint8_t idx, const meow_csp_reroute_entry_t* entry)
{
    if (!entry)
        return MEOW_CSP_ERR_NULL;

    if (idx >= MEOW_CSP_REROUTE_MAX)
        return MEOW_CSP_ERR_BOUNDS;

    if (reroute_table[idx].active)
        meow_csp_handler_clear(reroute_table[idx].dst_port);

    reroute_table[idx] = *entry;

    if (!entry->active)
        return MEOW_CSP_OK;

    return meow_csp_handler_set(entry->dst_port, reroute_fn, &reroute_table[idx]);
}

int meow_csp_reroute_clear(uint8_t idx)
{
    if (idx >= MEOW_CSP_REROUTE_MAX)
        return MEOW_CSP_ERR_BOUNDS;

    if (reroute_table[idx].active)
        meow_csp_handler_clear(reroute_table[idx].dst_port);

    memset(&reroute_table[idx], 0, sizeof(reroute_table[idx]));
    return MEOW_CSP_OK;
}
