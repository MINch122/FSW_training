#ifndef MEOW_CSP_EXT_STUBS_H
#define MEOW_CSP_EXT_STUBS_H

#include <stdint.h>
#include <csp/csp.h>
#include "gs/ftp/client.h"
#include "osapi-module.h"

/* -------------------------------------------------------------------------
 * Stub reset — call in test setup to restore all defaults
 * ---------------------------------------------------------------------- */

void Meow_CspExtStub_Reset(void);

/* -------------------------------------------------------------------------
 * CSP stub controls
 * ---------------------------------------------------------------------- */

extern int          Stub_csp_init_ret;
extern int          Stub_csp_route_start_task_ret;
extern csp_socket_t* Stub_csp_socket_ret;
extern int          Stub_csp_bind_ret;
extern int          Stub_csp_listen_ret;
extern uint8_t      Stub_csp_dport;
extern uint8_t      Stub_csp_src_node;
extern int          Stub_csp_sendto_ret;
extern int          Stub_csp_sendto_call_count;
extern csp_iface_t* Stub_csp_iflist_get_ret;
extern int          Stub_csp_rtable_set_ret;
extern int          Stub_csp_buffer_get_fail;
extern int          Stub_csp_service_handler_called;

/* -------------------------------------------------------------------------
 * OSAL module stub controls
 * ---------------------------------------------------------------------- */

extern int32      Stub_OS_ModuleLoad_ret;
extern osal_id_t  Stub_OS_ModuleLoad_id;
extern int32      Stub_OS_ModuleUnload_ret;
extern int        Stub_OS_ModuleUnload_call_count;
extern int32      Stub_OS_ModuleSymbolLookup_ret;
extern cpuaddr    Stub_OS_ModuleSymbolLookup_addr;

/* -------------------------------------------------------------------------
 * GS FTP stub controls
 * ---------------------------------------------------------------------- */

extern gs_error_t Stub_gs_ftp_upload_ret;
extern gs_error_t Stub_gs_ftp_download_ret;

/* Fake CSP objects tests can pass around */
extern csp_socket_t* Stub_csp_socket_default;
extern csp_iface_t   Stub_fake_iface;

#endif /* MEOW_CSP_EXT_STUBS_H */
