/**
 * Configurable stubs for CSP, OSAL module loader, and GS FTP.
 * Used exclusively by the meow_csp coverage test.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/csp_iflist.h>
#include <csp/csp_rtable.h>

#include "osapi-module.h"
#include "osapi-error.h"
#include "gs/ftp/client.h"

/* Complete the opaque csp_conn_s so we can create local instances */
struct csp_conn_s { int _; };

/* -------------------------------------------------------------------------
 * Stub control globals
 * ---------------------------------------------------------------------- */

int          Stub_csp_init_ret             = CSP_ERR_NONE;
int          Stub_csp_route_start_task_ret = CSP_ERR_NONE;
int          Stub_csp_bind_ret             = CSP_ERR_NONE;
int          Stub_csp_listen_ret           = CSP_ERR_NONE;
uint8_t      Stub_csp_dport                = 0;
uint8_t      Stub_csp_src_node             = 0;
int          Stub_csp_sendto_ret           = CSP_ERR_NONE;
int          Stub_csp_sendto_call_count    = 0;
csp_iface_t* Stub_csp_iflist_get_ret       = NULL;
int          Stub_csp_rtable_set_ret       = CSP_ERR_NONE;
int          Stub_csp_buffer_get_fail      = 0;
int          Stub_csp_service_handler_called = 0;

int32     Stub_OS_ModuleLoad_ret          = OS_SUCCESS;
osal_id_t Stub_OS_ModuleLoad_id;
int32     Stub_OS_ModuleUnload_ret        = OS_SUCCESS;
int       Stub_OS_ModuleUnload_call_count = 0;
int32     Stub_OS_ModuleSymbolLookup_ret  = OS_SUCCESS;
cpuaddr   Stub_OS_ModuleSymbolLookup_addr = 0;

gs_error_t Stub_gs_ftp_upload_ret   = GS_OK;
gs_error_t Stub_gs_ftp_download_ret = GS_OK;

static struct csp_conn_s _fake_sock_obj;
csp_socket_t* Stub_csp_socket_default = &_fake_sock_obj;
csp_socket_t* Stub_csp_socket_ret;

csp_iface_t Stub_fake_iface = {
    .name     = "STUB",
    .tx       = 10,
    .rx       = 20,
    .tx_error = 1,
    .rx_error = 2,
    .drop     = 3,
};

void Meow_CspExtStub_Reset(void)
{
    Stub_csp_init_ret              = CSP_ERR_NONE;
    Stub_csp_route_start_task_ret  = CSP_ERR_NONE;
    Stub_csp_socket_ret            = Stub_csp_socket_default;
    Stub_csp_bind_ret              = CSP_ERR_NONE;
    Stub_csp_listen_ret            = CSP_ERR_NONE;
    Stub_csp_dport                 = 0;
    Stub_csp_src_node              = 0;
    Stub_csp_sendto_ret            = CSP_ERR_NONE;
    Stub_csp_sendto_call_count     = 0;
    Stub_csp_iflist_get_ret        = NULL;
    Stub_csp_rtable_set_ret        = CSP_ERR_NONE;
    Stub_csp_buffer_get_fail       = 0;
    Stub_csp_service_handler_called = 0;

    Stub_OS_ModuleLoad_ret          = OS_SUCCESS;
    Stub_OS_ModuleUnload_ret        = OS_SUCCESS;
    Stub_OS_ModuleUnload_call_count = 0;
    Stub_OS_ModuleSymbolLookup_ret  = OS_SUCCESS;
    Stub_OS_ModuleSymbolLookup_addr = 0;

    Stub_gs_ftp_upload_ret   = GS_OK;
    Stub_gs_ftp_download_ret = GS_OK;
}

/* -------------------------------------------------------------------------
 * CSP stubs
 * ---------------------------------------------------------------------- */

int csp_init(const csp_conf_t* conf) { (void)conf; return Stub_csp_init_ret; }

int csp_route_start_task(unsigned int  stack_size, unsigned int  priority)
{
    (void)stack_size; (void)priority;
    return Stub_csp_route_start_task_ret;
}

csp_socket_t* csp_socket(uint32_t opts) { (void)opts; return Stub_csp_socket_ret; }

int csp_bind(csp_socket_t* sock, uint8_t port)
{
    (void)sock; (void)port;
    return Stub_csp_bind_ret;
}

int csp_listen(csp_socket_t* sock, size_t backlog)
{
    (void)sock; (void)backlog;
    return Stub_csp_listen_ret;
}

csp_conn_t* csp_accept(csp_socket_t* sock, uint32_t timeout_ms)
{
    (void)sock; (void)timeout_ms;
    return NULL;  /* always NULL — server thread loops and continues */
}

csp_packet_t* csp_read(csp_conn_t* conn, uint32_t timeout_ms)
{
    (void)conn; (void)timeout_ms;
    return NULL;
}

int csp_close(csp_conn_t* conn) { (void)conn; return CSP_ERR_NONE; }

int csp_conn_dport(csp_conn_t* conn) { (void)conn; return (int)Stub_csp_dport; }

int csp_conn_src(csp_conn_t* conn) { (void)conn; return (int)Stub_csp_src_node; }

void csp_service_handler(csp_conn_t* conn, csp_packet_t* packet)
{
    (void)conn; (void)packet;
    Stub_csp_service_handler_called++;
}

int csp_sendto(uint8_t prio, uint8_t dest, uint8_t dport, uint8_t sport,
               uint32_t opts, csp_packet_t* packet, uint32_t timeout_ms)
{
    (void)prio; (void)dest; (void)dport; (void)sport;
    (void)opts; (void)packet; (void)timeout_ms;
    Stub_csp_sendto_call_count++;
    return Stub_csp_sendto_ret;
}

void* csp_buffer_get(size_t data_size)
{
    if (Stub_csp_buffer_get_fail) return NULL;
    void* p = malloc(sizeof(csp_packet_t) + data_size);
    if (p) memset(p, 0, sizeof(csp_packet_t) + data_size);
    return p;
}

void csp_buffer_free(void* buf) { free(buf); }

csp_iface_t* csp_iflist_get_by_name(const char* name)
{
    (void)name;
    return Stub_csp_iflist_get_ret;
}

int csp_rtable_set(uint8_t node, uint8_t mask, csp_iface_t* iface, uint8_t via)
{
    (void)node; (void)mask; (void)iface; (void)via;
    return Stub_csp_rtable_set_ret;
}

/* -------------------------------------------------------------------------
 * OSAL module stubs
 * ---------------------------------------------------------------------- */

int32 OS_ModuleLoad(osal_id_t* module_id, const char* module_name,
                    const char* filename, uint32 flags)
{
    (void)module_name; (void)filename; (void)flags;
    if (module_id) *module_id = Stub_OS_ModuleLoad_id;
    return Stub_OS_ModuleLoad_ret;
}

int32 OS_ModuleSymbolLookup(osal_id_t module_id, cpuaddr* symbol_address,
                             const char* symbol_name)
{
    (void)module_id; (void)symbol_name;
    if (symbol_address) *symbol_address = Stub_OS_ModuleSymbolLookup_addr;
    return Stub_OS_ModuleSymbolLookup_ret;
}

int32 OS_ModuleUnload(osal_id_t module_id)
{
    (void)module_id;
    Stub_OS_ModuleUnload_call_count++;
    return Stub_OS_ModuleUnload_ret;
}

/* -------------------------------------------------------------------------
 * GS FTP stubs
 * ---------------------------------------------------------------------- */

gs_error_t gs_ftp_default_settings(gs_ftp_settings_t * settings)
{
    if (settings) memset(settings, 0, sizeof(*settings));
    return GS_OK;
}

gs_error_t gs_ftp_upload(const gs_ftp_settings_t * settings, const char * local_url, const char * remote_url,
                         gs_ftp_info_callback_t info_callback, void * info_data)
{
    (void)settings; (void)local_url; (void)remote_url; (void)info_callback; (void)info_data;
    return Stub_gs_ftp_upload_ret;
}

gs_error_t gs_ftp_download(const gs_ftp_settings_t * settings, const char * local_url,  const char * remote_url,
                           gs_ftp_info_callback_t info_callback, void * info_data)
{
    (void)settings; (void)local_url; (void)remote_url; (void)info_callback; (void)info_data;
    return Stub_gs_ftp_download_ret;
}
