/**
 * Coverage tests for meow_csp.c.
 * External CSP/OSAL/FTP calls are intercepted by meow_csp_ext_stubs.c.
 */

#include "utassert.h"
#include "uttest.h"

#include "meow_csp.h"
#include "meow_csp_ext_stubs.h"
#include "osapi-error.h"

#include <string.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Test-local handler tracking
 * ---------------------------------------------------------------------- */

static int handler_call_count = 0;
static int loaded_handler_call_count = 0;

static void test_handler(csp_conn_t* conn, csp_packet_t* packet, void* ctx)
{
    (void)conn; (void)ctx;
    handler_call_count++;
    csp_buffer_free(packet);
}

static void loaded_handler(csp_conn_t* conn, csp_packet_t* packet, void* ctx)
{
    (void)conn; (void)ctx;
    loaded_handler_call_count++;
    csp_buffer_free(packet);
}

/* -------------------------------------------------------------------------
 * Setup / teardown
 * ---------------------------------------------------------------------- */

static void Setup(void)
{
    Meow_CspExtStub_Reset();
    handler_call_count        = 0;
    loaded_handler_call_count = 0;
    /* Clear a broad port range so each test starts with a clean handler table */
    for (uint8_t p = 0; p < 24; p++)
        meow_csp_handler_clear(p);
}

static void Teardown(void) { /* nothing */ }

/* -------------------------------------------------------------------------
 * meow_csp_init
 * ---------------------------------------------------------------------- */

void Test_Init_Success(void)
{
    UtAssert_INT32_EQ(meow_csp_init(NULL), MEOW_CSP_OK);
}

void Test_Init_CspInitFail(void)
{
    Stub_csp_init_ret = -1;
    UtAssert_INT32_EQ(meow_csp_init(NULL), MEOW_CSP_ERR_INIT_CSP);
    UtAssert_INT32_EQ(meow_csp_last_err(), -1);
}

void Test_Init_RouterFail(void)
{
    Stub_csp_route_start_task_ret = -2;
    UtAssert_INT32_EQ(meow_csp_init(NULL), MEOW_CSP_ERR_INIT_TASK);
    UtAssert_INT32_EQ(meow_csp_last_err(), -2);
}

/* -------------------------------------------------------------------------
 * meow_csp_dispatch
 * ---------------------------------------------------------------------- */

void Test_Dispatch_FallsThrough_ToServiceHandler(void)
{
    Stub_csp_dport = 7;
    csp_packet_t* pkt = csp_buffer_get(4);
    UtAssert_True(pkt != NULL, "buffer allocated");

    meow_csp_dispatch(NULL, pkt);

    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 1);
    UtAssert_INT32_EQ(handler_call_count, 0);
    /* csp_service_handler stub does NOT free; packet leaks here in test — acceptable */
}

void Test_Dispatch_CallsCustomHandler(void)
{
    meow_csp_handler_set(10, test_handler, NULL);
    Stub_csp_dport = 10;

    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);

    UtAssert_INT32_EQ(handler_call_count, 1);
    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 0);

    meow_csp_handler_clear(10);
}

void Test_Dispatch_WrongPort_FallsThrough(void)
{
    meow_csp_handler_set(11, test_handler, NULL);
    Stub_csp_dport = 12;  /* different port */

    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);

    UtAssert_INT32_EQ(handler_call_count, 0);
    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 1);

    meow_csp_handler_clear(11);
}

/* -------------------------------------------------------------------------
 * meow_csp_handler_set
 * ---------------------------------------------------------------------- */

void Test_HandlerSet_NullFn(void)
{
    UtAssert_INT32_EQ(meow_csp_handler_set(5, NULL, NULL), MEOW_CSP_ERR_NULL);
}

void Test_HandlerSet_Register(void)
{
    UtAssert_INT32_EQ(meow_csp_handler_set(5, test_handler, NULL), MEOW_CSP_OK);
    meow_csp_handler_clear(5);
}

void Test_HandlerSet_Replace(void)
{
    meow_csp_handler_set(5, test_handler, NULL);
    UtAssert_INT32_EQ(meow_csp_handler_set(5, test_handler, (void*)1), MEOW_CSP_OK);
    meow_csp_handler_clear(5);
}

void Test_HandlerSet_TableFull(void)
{
    /* Fill 8 slots with unique ports */
    for (uint8_t p = 100; p < 108; p++)
        meow_csp_handler_set(p, test_handler, NULL);

    /* 9th registration should fail */
    UtAssert_INT32_EQ(meow_csp_handler_set(108, test_handler, NULL), MEOW_CSP_ERR_BOUNDS);

    for (uint8_t p = 100; p < 108; p++)
        meow_csp_handler_clear(p);
}

/* -------------------------------------------------------------------------
 * meow_csp_handler_clear
 * ---------------------------------------------------------------------- */

void Test_HandlerClear_Registered(void)
{
    meow_csp_handler_set(15, test_handler, NULL);
    UtAssert_INT32_EQ(meow_csp_handler_clear(15), MEOW_CSP_OK);

    /* After clearing, dispatch should fall through to service handler */
    Stub_csp_dport = 15;
    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);
    UtAssert_INT32_EQ(handler_call_count, 0);
    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 1);
}

void Test_HandlerClear_NotRegistered_IsNoOp(void)
{
    UtAssert_INT32_EQ(meow_csp_handler_clear(20), MEOW_CSP_OK);
}

/* -------------------------------------------------------------------------
 * meow_csp_handler_load
 * ---------------------------------------------------------------------- */

void Test_HandlerLoad_NullPath(void)
{
    UtAssert_INT32_EQ(meow_csp_handler_load(5, NULL, "sym"), MEOW_CSP_ERR_NULL);
}

void Test_HandlerLoad_NullSymbol(void)
{
    UtAssert_INT32_EQ(meow_csp_handler_load(5, "/some/path.so", NULL), MEOW_CSP_ERR_NULL);
}

void Test_HandlerLoad_ModuleLoadFail(void)
{
    Stub_OS_ModuleLoad_ret = -1;
    UtAssert_INT32_EQ(meow_csp_handler_load(5, "/path.so", "sym"), MEOW_CSP_ERR_MODULE);
    UtAssert_INT32_EQ(meow_csp_last_module_err(), -1);
}

void Test_HandlerLoad_SymbolLookupFail(void)
{
    Stub_OS_ModuleLoad_ret         = OS_SUCCESS;
    Stub_OS_ModuleSymbolLookup_ret = -2;

    UtAssert_INT32_EQ(meow_csp_handler_load(5, "/path.so", "sym"), MEOW_CSP_ERR_MODULE);
    /* module must have been unloaded after lookup failure */
    UtAssert_INT32_EQ(Stub_OS_ModuleUnload_call_count, 1);
    UtAssert_INT32_EQ(meow_csp_last_module_err(), -2);
}

void Test_HandlerLoad_Success_DispatchCallsIt(void)
{
    Stub_OS_ModuleSymbolLookup_addr = (cpuaddr)(uintptr_t)loaded_handler;

    UtAssert_INT32_EQ(meow_csp_handler_load(16, "/path.so", "loaded_handler"), MEOW_CSP_OK);

    Stub_csp_dport = 16;
    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);

    UtAssert_INT32_EQ(loaded_handler_call_count, 1);
    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 0);

    meow_csp_handler_clear(16);
    UtAssert_INT32_EQ(Stub_OS_ModuleUnload_call_count, 1);
}

void Test_HandlerLoad_ReplacesExisting_UnloadsOld(void)
{
    Stub_OS_ModuleSymbolLookup_addr = (cpuaddr)(uintptr_t)loaded_handler;

    meow_csp_handler_load(17, "/path1.so", "fn");
    int unloads_after_first = Stub_OS_ModuleUnload_call_count;

    meow_csp_handler_load(17, "/path2.so", "fn");
    UtAssert_True(Stub_OS_ModuleUnload_call_count > unloads_after_first,
                  "old module unloaded on replacement");

    meow_csp_handler_clear(17);
}

/* -------------------------------------------------------------------------
 * meow_csp_send
 * ---------------------------------------------------------------------- */

void Test_Send_NullData(void)
{
    UtAssert_INT32_EQ(meow_csp_send(2, 5, 6, 0, NULL, 10, 1000), MEOW_CSP_ERR_SEND);
}

void Test_Send_ZeroLen(void)
{
    uint8_t d = 0;
    UtAssert_INT32_EQ(meow_csp_send(2, 5, 6, 0, &d, 0, 1000), MEOW_CSP_ERR_SEND);
}

void Test_Send_NoBuffer(void)
{
    uint8_t d = 0xAB;
    Stub_csp_buffer_get_fail = 1;
    UtAssert_INT32_EQ(meow_csp_send(2, 5, 6, 0, &d, 1, 1000), MEOW_CSP_ERR_NOMEM);
}

void Test_Send_SendtoFail(void)
{
    uint8_t d = 0xAB;
    Stub_csp_sendto_ret = -3;
    UtAssert_INT32_EQ(meow_csp_send(2, 5, 6, 0, &d, 1, 1000), MEOW_CSP_ERR_SEND);
    UtAssert_INT32_EQ(meow_csp_last_err(), -3);
}

void Test_Send_Success(void)
{
    uint8_t d = 0xAB;
    UtAssert_INT32_EQ(meow_csp_send(2, 5, 6, 0, &d, 1, 1000), MEOW_CSP_OK);
    UtAssert_INT32_EQ(Stub_csp_sendto_call_count, 1);
}

/* -------------------------------------------------------------------------
 * meow_csp_ftp_upload
 * ---------------------------------------------------------------------- */

void Test_FtpUpload_NullLocal(void)
{
    UtAssert_INT32_EQ(meow_csp_ftp_upload(3, 0, NULL, "/remote", 0, 0), MEOW_CSP_ERR_NULL);
}

void Test_FtpUpload_NullRemote(void)
{
    UtAssert_INT32_EQ(meow_csp_ftp_upload(3, 0, "/local", NULL, 0, 0), MEOW_CSP_ERR_NULL);
}

void Test_FtpUpload_Fail(void)
{
    Stub_gs_ftp_upload_ret = -1;
    UtAssert_INT32_EQ(meow_csp_ftp_upload(3, 0, "/l", "/r", 0, 0), MEOW_CSP_ERR_FTP);
    UtAssert_INT32_EQ(meow_csp_last_ftp_err(), -1);
}

void Test_FtpUpload_Success(void)
{
    UtAssert_INT32_EQ(meow_csp_ftp_upload(3, 0, "/l", "/r", 0, 0), MEOW_CSP_OK);
}

/* -------------------------------------------------------------------------
 * meow_csp_ftp_download
 * ---------------------------------------------------------------------- */

void Test_FtpDownload_NullLocal(void)
{
    UtAssert_INT32_EQ(meow_csp_ftp_download(3, 0, NULL, "/remote", 0, 0), MEOW_CSP_ERR_NULL);
}

void Test_FtpDownload_NullRemote(void)
{
    UtAssert_INT32_EQ(meow_csp_ftp_download(3, 0, "/local", NULL, 0, 0), MEOW_CSP_ERR_NULL);
}

void Test_FtpDownload_Fail(void)
{
    Stub_gs_ftp_download_ret = -4;
    UtAssert_INT32_EQ(meow_csp_ftp_download(3, 0, "/l", "/r", 0, 0), MEOW_CSP_ERR_FTP);
    UtAssert_INT32_EQ(meow_csp_last_ftp_err(), -4);
}

void Test_FtpDownload_Success(void)
{
    UtAssert_INT32_EQ(meow_csp_ftp_download(3, 0, "/l", "/r", 0, 0), MEOW_CSP_OK);
}

/* -------------------------------------------------------------------------
 * meow_csp_ifstats
 * ---------------------------------------------------------------------- */

void Test_Ifstats_NullArgs(void)
{
    meow_csp_ifstats_t s;
    UtAssert_INT32_EQ(meow_csp_ifstats(NULL, &s),      MEOW_CSP_ERR_NULL);
    UtAssert_INT32_EQ(meow_csp_ifstats("LOOP", NULL),  MEOW_CSP_ERR_NULL);
}

void Test_Ifstats_IfaceNotFound(void)
{
    meow_csp_ifstats_t s;
    Stub_csp_iflist_get_ret = NULL;
    UtAssert_INT32_EQ(meow_csp_ifstats("LOOP", &s), MEOW_CSP_ERR_IFACE);
}

void Test_Ifstats_Success(void)
{
    Stub_fake_iface.tx       = 100;
    Stub_fake_iface.rx       = 200;
    Stub_fake_iface.tx_error = 3;
    Stub_fake_iface.rx_error = 4;
    Stub_fake_iface.drop     = 5;
    Stub_csp_iflist_get_ret  = &Stub_fake_iface;

    meow_csp_ifstats_t s = {0};
    UtAssert_INT32_EQ(meow_csp_ifstats("STUB", &s), MEOW_CSP_OK);
    UtAssert_UINT32_EQ(s.tx, 100);
    UtAssert_UINT32_EQ(s.rx, 200);
    UtAssert_UINT32_EQ(s.tx_error, 3);
    UtAssert_UINT32_EQ(s.rx_error, 4);
    UtAssert_UINT32_EQ(s.drop, 5);
}

/* -------------------------------------------------------------------------
 * meow_csp_route_set
 * ---------------------------------------------------------------------- */

void Test_RouteSet_NullIfaceName(void)
{
    UtAssert_INT32_EQ(meow_csp_route_set(2, 8, NULL, 0), MEOW_CSP_ERR_NULL);
}

void Test_RouteSet_IfaceNotFound(void)
{
    Stub_csp_iflist_get_ret = NULL;
    UtAssert_INT32_EQ(meow_csp_route_set(2, 8, "LOOP", 0), MEOW_CSP_ERR_IFACE);
}

void Test_RouteSet_RtableFail(void)
{
    Stub_csp_iflist_get_ret = &Stub_fake_iface;
    Stub_csp_rtable_set_ret = -5;
    UtAssert_INT32_EQ(meow_csp_route_set(2, 8, "STUB", 0), MEOW_CSP_ERR_ROUTE);
    UtAssert_INT32_EQ(meow_csp_last_err(), -5);
}

void Test_RouteSet_Success(void)
{
    Stub_csp_iflist_get_ret = &Stub_fake_iface;
    UtAssert_INT32_EQ(meow_csp_route_set(2, 8, "STUB", 0), MEOW_CSP_OK);
}

/* -------------------------------------------------------------------------
 * meow_csp_reroute_set / reroute_clear
 * ---------------------------------------------------------------------- */

void Test_RerouteSet_OutOfBounds(void)
{
    meow_csp_reroute_entry_t e = {0};
    UtAssert_INT32_EQ(meow_csp_reroute_set(MEOW_CSP_REROUTE_MAX, &e), MEOW_CSP_ERR_BOUNDS);
}

void Test_RerouteSet_NullEntry(void)
{
    UtAssert_INT32_EQ(meow_csp_reroute_set(0, NULL), MEOW_CSP_ERR_NULL);
}

void Test_RerouteSet_Active_DispatchForwards(void)
{
    meow_csp_reroute_entry_t rule = {
        .dst_port    = 22,
        .fwd_dst     = 3,
        .fwd_dst_port= 8,
        .fwd_src_port= 1,
        .timeout_ms  = 0,
        .active      = 1,
    };
    UtAssert_INT32_EQ(meow_csp_reroute_set(0, &rule), MEOW_CSP_OK);

    Stub_csp_dport = 22;
    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);

    /* reroute_fn calls csp_sendto and frees packet on success */
    UtAssert_INT32_EQ(Stub_csp_sendto_call_count, 1);
    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 0);

    meow_csp_reroute_clear(0);
}

void Test_RerouteSet_Inactive_DoesNotRegisterHandler(void)
{
    meow_csp_reroute_entry_t rule = {
        .dst_port = 23,
        .active   = 0,
    };
    meow_csp_reroute_set(1, &rule);

    Stub_csp_dport = 23;
    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);

    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 1); /* fell through */
    meow_csp_reroute_clear(1);
}

void Test_RerouteClear_OutOfBounds(void)
{
    UtAssert_INT32_EQ(meow_csp_reroute_clear(MEOW_CSP_REROUTE_MAX), MEOW_CSP_ERR_BOUNDS);
}

void Test_RerouteClear_Success(void)
{
    meow_csp_reroute_entry_t rule = { .dst_port = 24, .active = 1 };
    meow_csp_reroute_set(2, &rule);
    UtAssert_INT32_EQ(meow_csp_reroute_clear(2), MEOW_CSP_OK);

    /* after clear, port 24 should fall through */
    Stub_csp_dport = 24;
    csp_packet_t* pkt = csp_buffer_get(4);
    meow_csp_dispatch(NULL, pkt);
    UtAssert_INT32_EQ(Stub_csp_service_handler_called, 1);
}

/* -------------------------------------------------------------------------
 * meow_csp_server_start / server_stop
 * ---------------------------------------------------------------------- */

void Test_Server_Start_SocketFail(void)
{
    Stub_csp_socket_ret = NULL;
    UtAssert_INT32_EQ(meow_csp_server_start(), MEOW_CSP_ERR_SOCKET);
}

void Test_Server_Start_BindFail(void)
{
    Stub_csp_bind_ret = -1;
    UtAssert_INT32_EQ(meow_csp_server_start(), MEOW_CSP_ERR_SOCKET);
}

void Test_Server_Start_ListenFail(void)
{
    Stub_csp_listen_ret = -1;
    UtAssert_INT32_EQ(meow_csp_server_start(), MEOW_CSP_ERR_SOCKET);
}

void Test_Server_StartStop(void)
{
    UtAssert_INT32_EQ(meow_csp_server_start(), MEOW_CSP_OK);
    UtAssert_INT32_EQ(meow_csp_server_stop(),  MEOW_CSP_OK);
}

void Test_Server_DoubleStart(void)
{
    meow_csp_server_start();
    UtAssert_INT32_EQ(meow_csp_server_start(), MEOW_CSP_ERR_THREAD);
    meow_csp_server_stop();
}

void Test_Server_Stop_NotRunning(void)
{
    UtAssert_INT32_EQ(meow_csp_server_stop(), MEOW_CSP_OK);
}

/* -------------------------------------------------------------------------
 * Error accessors
 * ---------------------------------------------------------------------- */

void Test_LastErrAccessors(void)
{
    Stub_csp_init_ret = -7;
    meow_csp_init(NULL);
    UtAssert_INT32_EQ(meow_csp_last_err(), -7);

    Stub_gs_ftp_upload_ret = -8;
    meow_csp_ftp_upload(0, 0, "/l", "/r", 0, 0);
    UtAssert_INT32_EQ(meow_csp_last_ftp_err(), -8);

    Stub_OS_ModuleLoad_ret = -9;
    meow_csp_handler_load(5, "/p", "s");
    UtAssert_INT32_EQ(meow_csp_last_module_err(), -9);
}

/* -------------------------------------------------------------------------
 * Test registration
 * ---------------------------------------------------------------------- */

void UtTest_Setup(void)
{
    UtTest_Add(Test_Init_Success,          Setup, Teardown, "Init_Success");
    UtTest_Add(Test_Init_CspInitFail,      Setup, Teardown, "Init_CspInitFail");
    UtTest_Add(Test_Init_RouterFail,       Setup, Teardown, "Init_RouterFail");

    UtTest_Add(Test_Dispatch_FallsThrough_ToServiceHandler,
               Setup, Teardown, "Dispatch_FallsThrough");
    UtTest_Add(Test_Dispatch_CallsCustomHandler,
               Setup, Teardown, "Dispatch_CallsCustomHandler");
    UtTest_Add(Test_Dispatch_WrongPort_FallsThrough,
               Setup, Teardown, "Dispatch_WrongPort");

    UtTest_Add(Test_HandlerSet_NullFn,     Setup, Teardown, "HandlerSet_NullFn");
    UtTest_Add(Test_HandlerSet_Register,   Setup, Teardown, "HandlerSet_Register");
    UtTest_Add(Test_HandlerSet_Replace,    Setup, Teardown, "HandlerSet_Replace");
    UtTest_Add(Test_HandlerSet_TableFull,  Setup, Teardown, "HandlerSet_TableFull");

    UtTest_Add(Test_HandlerClear_Registered,      Setup, Teardown, "HandlerClear_Registered");
    UtTest_Add(Test_HandlerClear_NotRegistered_IsNoOp,
               Setup, Teardown, "HandlerClear_NoOp");

    UtTest_Add(Test_HandlerLoad_NullPath,         Setup, Teardown, "HandlerLoad_NullPath");
    UtTest_Add(Test_HandlerLoad_NullSymbol,       Setup, Teardown, "HandlerLoad_NullSymbol");
    UtTest_Add(Test_HandlerLoad_ModuleLoadFail,   Setup, Teardown, "HandlerLoad_ModuleLoadFail");
    UtTest_Add(Test_HandlerLoad_SymbolLookupFail, Setup, Teardown, "HandlerLoad_SymbolFail");
    UtTest_Add(Test_HandlerLoad_Success_DispatchCallsIt,
               Setup, Teardown, "HandlerLoad_Success");
    UtTest_Add(Test_HandlerLoad_ReplacesExisting_UnloadsOld,
               Setup, Teardown, "HandlerLoad_Replace");

    UtTest_Add(Test_Send_NullData,         Setup, Teardown, "Send_NullData");
    UtTest_Add(Test_Send_ZeroLen,          Setup, Teardown, "Send_ZeroLen");
    UtTest_Add(Test_Send_NoBuffer,         Setup, Teardown, "Send_NoBuffer");
    UtTest_Add(Test_Send_SendtoFail,       Setup, Teardown, "Send_SendtoFail");
    UtTest_Add(Test_Send_Success,          Setup, Teardown, "Send_Success");

    UtTest_Add(Test_FtpUpload_NullLocal,   Setup, Teardown, "FtpUpload_NullLocal");
    UtTest_Add(Test_FtpUpload_NullRemote,  Setup, Teardown, "FtpUpload_NullRemote");
    UtTest_Add(Test_FtpUpload_Fail,        Setup, Teardown, "FtpUpload_Fail");
    UtTest_Add(Test_FtpUpload_Success,     Setup, Teardown, "FtpUpload_Success");

    UtTest_Add(Test_FtpDownload_NullLocal, Setup, Teardown, "FtpDownload_NullLocal");
    UtTest_Add(Test_FtpDownload_NullRemote,Setup, Teardown, "FtpDownload_NullRemote");
    UtTest_Add(Test_FtpDownload_Fail,      Setup, Teardown, "FtpDownload_Fail");
    UtTest_Add(Test_FtpDownload_Success,   Setup, Teardown, "FtpDownload_Success");

    UtTest_Add(Test_Ifstats_NullArgs,      Setup, Teardown, "Ifstats_NullArgs");
    UtTest_Add(Test_Ifstats_IfaceNotFound, Setup, Teardown, "Ifstats_IfaceNotFound");
    UtTest_Add(Test_Ifstats_Success,       Setup, Teardown, "Ifstats_Success");

    UtTest_Add(Test_RouteSet_NullIfaceName,Setup, Teardown, "RouteSet_NullIfaceName");
    UtTest_Add(Test_RouteSet_IfaceNotFound,Setup, Teardown, "RouteSet_IfaceNotFound");
    UtTest_Add(Test_RouteSet_RtableFail,   Setup, Teardown, "RouteSet_RtableFail");
    UtTest_Add(Test_RouteSet_Success,      Setup, Teardown, "RouteSet_Success");

    UtTest_Add(Test_RerouteSet_OutOfBounds,           Setup, Teardown, "RerouteSet_Bounds");
    UtTest_Add(Test_RerouteSet_NullEntry,             Setup, Teardown, "RerouteSet_NullEntry");
    UtTest_Add(Test_RerouteSet_Active_DispatchForwards,
               Setup, Teardown, "RerouteSet_Active");
    UtTest_Add(Test_RerouteSet_Inactive_DoesNotRegisterHandler,
               Setup, Teardown, "RerouteSet_Inactive");
    UtTest_Add(Test_RerouteClear_OutOfBounds,         Setup, Teardown, "RerouteClear_Bounds");
    UtTest_Add(Test_RerouteClear_Success,             Setup, Teardown, "RerouteClear_Success");

    UtTest_Add(Test_Server_Start_SocketFail, Setup, Teardown, "Server_SocketFail");
    UtTest_Add(Test_Server_Start_BindFail,   Setup, Teardown, "Server_BindFail");
    UtTest_Add(Test_Server_Start_ListenFail, Setup, Teardown, "Server_ListenFail");
    UtTest_Add(Test_Server_StartStop,        Setup, Teardown, "Server_StartStop");
    UtTest_Add(Test_Server_DoubleStart,      Setup, Teardown, "Server_DoubleStart");
    UtTest_Add(Test_Server_Stop_NotRunning,  Setup, Teardown, "Server_StopNotRunning");

    UtTest_Add(Test_LastErrAccessors,        Setup, Teardown, "LastErrAccessors");
}
