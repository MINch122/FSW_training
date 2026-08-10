/**
 * @file meow_csp.h
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: CSP (CubeSat Space Protocol) wrapper utilities.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#ifndef _MEOW_CSP_H_
#define _MEOW_CSP_H_

#ifdef MEOW_INCLUDE_CSP

#include <stdint.h>
#include <csp/csp.h>


/* return codes */
typedef enum {
    MEOW_CSP_OK            =  0,
    MEOW_CSP_ERR_NULL      = -1,
    MEOW_CSP_ERR_INIT_CSP  = -2,
    MEOW_CSP_ERR_INIT_TASK = -3,
    MEOW_CSP_ERR_SOCKET    = -4,
    MEOW_CSP_ERR_SEND      = -5,
    MEOW_CSP_ERR_FTP       = -6,
    MEOW_CSP_ERR_NOMEM     = -7,
    MEOW_CSP_ERR_ROUTE     = -8,
    MEOW_CSP_ERR_IFACE     = -9,
    MEOW_CSP_ERR_THREAD    = -10,
    MEOW_CSP_ERR_BOUNDS    = -11,   /* index out of range or handler table full */
    MEOW_CSP_ERR_MODULE    = -12,  /* OS_ModuleLoad or OS_ModuleSymbolLookup failed */
} meow_csp_ret_t;


#define MEOW_CSP_REROUTE_MAX 8   /* max reroute table entries */
#define MEOW_CSP_HANDLER_MAX 8   /* max custom port handler entries */


/**
 * @brief Callback type for custom per-port packet handlers.
 *
 * @details
 *      - The callee takes ownership of packet: it must either send it
 *            (transferring ownership to the CSP stack) or free it via
 *            csp_buffer_free(). Leaking the packet stalls the buffer pool.
 *      - ctx is the pointer passed to meow_csp_handler_set() and may be NULL.
 */
typedef void (*meow_csp_port_handler)(csp_conn_t* conn,
                                      csp_packet_t* packet,
                                      void* ctx);

typedef struct {
    uint32_t tx;
    uint32_t rx;
    uint32_t tx_error;
    uint32_t rx_error;
    uint32_t drop;
} meow_csp_ifstats_t;

/**
 * @brief One entry in the static reroute table.
 *
 * @details
 *      - When a connection arrives whose destination port equals dst_port,
 *            every packet on that connection is forwarded to fwd_dst:fwd_dst_port
 *            instead of being passed to csp_service_handler.
 *      - If src_node is non-zero, only packets whose source address matches
 *            src_node are forwarded; all others fall through to csp_service_handler.
 *      - timeout_ms == 0 uses a 1000 ms default.
 *      - active == 0 disables the entry without clearing the other fields.
 */
typedef struct {
    uint8_t  dst_port;      /* intercept connections arriving on this port          */
    uint8_t  src_node;      /* filter: 0 = any source; non-zero = match only this   */
    uint8_t  fwd_dst;       /* forward to this CSP node address                     */
    uint8_t  fwd_dst_port;  /* ... on this port                                     */
    uint8_t  fwd_src_port;  /* using this source port                               */
    uint8_t  _pad[3];
    uint32_t timeout_ms;    /* per-packet send timeout (0 → 1000 ms)                */
    int      active;
} meow_csp_reroute_entry_t;

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

/**
 * @brief Initialise CSP, register interface, load routing table, start router task.
 *
 * @details
 *      - conf may be NULL; a built-in default (address=1, loopback only) is used.
 *      - On failure the raw CSP error code is saved and readable via meow_csp_last_err().
 *
 * @param conf  Node configuration, or NULL for defaults.
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_INIT_CSP if csp_init() fails.
 *          MEOW_CSP_ERR_INIT_TASK if csp_route_start_task() fails.
 */
int meow_csp_init(const csp_conf_t* conf);

/* -------------------------------------------------------------------------
 * Server thread
 * ---------------------------------------------------------------------- */

/**
 * @brief Create the server socket and spawn the server thread.
 *
 * @details
 *      - The server thread loops indefinitely on csp_accept(). For each
 *            accepted connection it reads all packets and routes them through
 *            meow_csp_dispatch().
 *      - Calling a second time while the thread is already running returns
 *            MEOW_CSP_ERR_THREAD without creating a second thread.
 *
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_SOCKET if socket creation, bind, or listen fails.
 *          MEOW_CSP_ERR_THREAD if pthread_create fails.
 */
int meow_csp_server_start(void);

/**
 * @brief Signal the server thread to stop and block until it exits.
 *
 * @details
 *      - Sets the stop flag and calls pthread_join(). The thread will finish
 *            the current accept cycle (up to 1 second) before exiting.
 *      - No-op if the server was never started (returns MEOW_CSP_OK).
 *
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_THREAD if pthread_join fails.
 */
int meow_csp_server_stop(void);

/**
 * @brief Dispatch a single CSP packet to the registered handler or csp_service_handler.
 *
 * @details
 *      Exposed for unit testing; the server thread calls this for every packet
 *      it reads. Custom handlers registered via meow_csp_handler_set() take
 *      priority; unmatched packets fall through to csp_service_handler().
 *
 * @param conn    Connection the packet arrived on.
 * @param packet  Packet to dispatch (ownership transferred to callee).
 */
void meow_csp_dispatch(csp_conn_t* conn, csp_packet_t* packet);

/**
 * @brief Set millisecond timeout for csp_read() calls in the server thread.
 * 
 * @param timeout_ms Timeout in milliseconds.
 */
void meow_csp_set_read_timeout(uint32_t timeout_ms);

/**
 * @brief Get the current millisecond timeout for csp_read() calls in the server thread.
 *        Default is 500 ms.
 * 
 * @return Current timeout in milliseconds.
 */
uint32_t meow_csp_get_read_timeout(void);

/* -------------------------------------------------------------------------
 * Custom dispatch
 * ---------------------------------------------------------------------- */

/**
 * @brief Register a custom handler for a specific destination port.
 *
 * @details
 *      - If a handler is already registered for port, it is replaced.
 *      - Thread-safe; may be called while the server thread is running.
 *      - fn must not be NULL.
 *      - fn and ctx are stored as-is addresses. If used, ctx must
 *            persist as long as the handler is registered.
 *
 * @param port  Destination port to intercept.
 * @param fn    Handler callback; takes ownership of each packet it receives.
 * @param ctx   Opaque pointer passed to fn on every call (may be NULL).
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_NULL if @a fn is NULL.
 *          MEOW_CSP_ERR_BOUNDS if the handler table is full.
 */
int meow_csp_handler_set(uint8_t port, meow_csp_port_handler fn, void* ctx);

/**
 * @brief Load a shared object, look up a symbol, and register it as a port handler.
 *
 * @details
 *      - Calls OS_ModuleLoad() with OS_MODULE_FLAG_LOCAL_SYMBOLS so the loaded
 *            symbols do not pollute the global namespace.
 *      - Calls OS_ModuleSymbolLookup() to resolve symbol to a function pointer.
 *      - If a handler is already registered on port, it is replaced: the old
 *            module is unloaded before the new one is attached.
 *      - The module lifetime is tied to the handler registration: clearing the
 *            handler via meow_csp_handler_clear() will call OS_ModuleUnload().
 *      - On any failure after OS_ModuleLoad() succeeds, the module is unloaded
 *            before returning.
 *      - Raw OSAL error codes on failure are readable via meow_csp_last_module_err().
 *
 * @param port    Destination port to intercept.
 * @param path    Filesystem path to the .so file.
 * @param symbol  Name of the handler symbol to look up
 *                (must match meow_csp_port_handler_fn signature).
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_NULL if @a path or @a symbol is NULL.
 *          MEOW_CSP_ERR_BOUNDS if the handler table is full.
 *          MEOW_CSP_ERR_MODULE if OS_ModuleLoad() or OS_ModuleSymbolLookup() fails.
 */
int meow_csp_handler_load(uint8_t port, const char* path, const char* symbol);

/**
 * @brief Remove the custom handler registered for a port.
 *
 * @details
 *      - If the handler was registered via meow_csp_handler_load(), the
 *            backing module is also unloaded via OS_ModuleUnload().
 *      - No-op if no handler is registered for port.
 *      - Thread-safe; may be called while the server thread is running.
 *
 * @param port  Destination port whose handler should be removed.
 * @return  Always MEOW_CSP_OK.
 */
int meow_csp_handler_clear(uint8_t port);

/* -------------------------------------------------------------------------
 * Send
 * ---------------------------------------------------------------------- */

/**
 * @brief Send a raw CSP packet connectionless.
 *
 * @details
 *      - Connectionless send: calls csp_sendto() with CSP_SO_NONE.
 *      - On failure the raw CSP error code is saved and readable via
 *            meow_csp_last_err().
 *
 * @param dst         Destination node address.
 * @param dst_port    Destination port.
 * @param src_port    Source port on this node.
 * @param prio        Priority (CSP_PRIO_*).
 * @param data        Payload bytes.
 * @param len         Payload length in bytes.
 * @param timeout_ms  Send timeout in milliseconds.
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_SEND if data is NULL or len is 0.
 *          MEOW_CSP_ERR_NOMEM if no packet buffer is available.
 *          MEOW_CSP_ERR_SEND if csp_sendto() fails.
 */
int meow_csp_send(uint8_t dst, uint8_t dst_port, uint8_t src_port,
                  uint8_t prio, const void* data, uint16_t len,
                  uint32_t timeout_ms);

/* -------------------------------------------------------------------------
 * FTP
 * ---------------------------------------------------------------------- */
  
/**
 * @brief Upload a local file to a remote CSP/FTP host.
 *
 * @details
 *      - On failure the raw GS FTP error code is saved and readable via
 *            meow_csp_last_ftp_err().
 *      - Upload in the view of the satellite: this is a downlink operation
 *            from local_path (onboard) to remote_path (ground).
 *
 * @param host        Destination CSP node address.
 * @param port        FTP port (0 → library default).
 * @param local_path  Local filesystem path (source).
 * @param remote_path Remote path on host (destination).
 * @param timeout_ms  Per-chunk timeout in ms (0 → library default).
 * @param chunk_size  Transfer chunk size in bytes (0 → library default).
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_NULL if @a local_path or @a remote_path is NULL.
 *          MEOW_CSP_ERR_FTP if transfer fails.
 */
int meow_csp_ftp_upload(uint8_t host, uint8_t port,
                        const char* local_path, const char* remote_path,
                        uint32_t timeout_ms, uint32_t chunk_size);

/**
 * @brief Download a remote file from a CSP/FTP host to local filesystem.
 *
 * @details
 *      - On failure the raw GS FTP error code is saved and readable via
 *            meow_csp_last_ftp_err().
 *      - Download in the view of the satellite: this is an uplink operation
 *            from remote_path (ground) to local_path (onboard).
 *
 * @param host        Source CSP node address.
 * @param port        FTP port (0 → library default).
 * @param local_path  Local filesystem path (destination).
 * @param remote_path Remote path on host (source).
 * @param timeout_ms  Per-chunk timeout in ms (0 → library default).
 * @param chunk_size  Transfer chunk size in bytes (0 → library default).
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_NULL if @a local_path or @a remote_path is NULL.
 *          MEOW_CSP_ERR_FTP if transfer fails.
 */
int meow_csp_ftp_download(uint8_t host, uint8_t port,
                          const char* local_path, const char* remote_path,
                          uint32_t timeout_ms, uint32_t chunk_size);

/* -------------------------------------------------------------------------
 * Interface stats / routing
 * ---------------------------------------------------------------------- */

/**
 * @brief Read TX/RX/error/drop counters for a named interface.
 *
 * @param iface_name  Interface name (e.g. "CAN", "UART", "LOOP").
 * @param[out] stats  Filled on MEOW_CSP_OK.
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_NULL if @a iface_name or @a stats is NULL.
 *          MEOW_CSP_ERR_IFACE if the interface is not found.
 */
int meow_csp_ifstats(const char* iface_name, meow_csp_ifstats_t* stats);

/**
 * @brief Add or update a routing table entry at runtime.
 *
 * @details
 *      - On failure the raw CSP error code is saved and readable via
 *            meow_csp_last_err().
 *
 * @param dst         Destination address.
 * @param mask        Address mask bits.
 * @param iface_name  Interface name to route through.
 * @param via         Next-hop address (CSP_NO_VIA_ADDRESS for direct).
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_NULL if @a iface_name is NULL.
 *          MEOW_CSP_ERR_ROUTE if csp_rtable_set() fails.
 *          MEOW_CSP_ERR_IFACE if the named interface is not found.
 */
int meow_csp_route_set(uint8_t dst, uint8_t mask,
                       const char* iface_name, uint8_t via);

/* -------------------------------------------------------------------------
 * Reroute table
 * ---------------------------------------------------------------------- */

/**
 * @brief Install or update a reroute table entry.
 *
 * @details
 *      - Thread-safe; may be called while the server thread is running.
 *      - Setting entry.active = 0 disables the rule without removing it.
 *
 * @param idx    Table slot (0 .. MEOW_CSP_REROUTE_MAX-1).
 * @param entry  Entry to copy into the table. Must be non-NULL.
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_BOUNDS if idx >= MEOW_CSP_REROUTE_MAX.
 *          MEOW_CSP_ERR_NULL if @a entry is NULL.
 */
int meow_csp_reroute_set(uint8_t idx, const meow_csp_reroute_entry_t* entry);

/**
 * @brief Disable and zero a reroute table entry.
 *
 * @details
 *      - Thread-safe; may be called while the server thread is running.
 *
 * @param idx  Table slot (0 .. MEOW_CSP_REROUTE_MAX-1).
 * @return  MEOW_CSP_OK on success.
 *          MEOW_CSP_ERR_BOUNDS if idx >= MEOW_CSP_REROUTE_MAX.
 */
int meow_csp_reroute_clear(uint8_t idx);

/* -------------------------------------------------------------------------
 * Raw error accessors
 * ---------------------------------------------------------------------- */

/**
 * @brief Return the raw CSP error code from the most recent CSP API failure.
 *
 * @details
 *      - Updated by meow_csp_init, meow_csp_send, meow_csp_route_set, and
 *            the server thread on forward failures.
 *      - Returns 0 (CSP_ERR_NONE) if no failure has occurred yet.
 */
int meow_csp_last_err(void);

/**
 * @brief Return the raw GS FTP error code from the most recent FTP failure.
 *
 * @details
 *      - Updated by meow_csp_ftp_upload and meow_csp_ftp_download on failure.
 *      - Returns 0 (GS_ERROR_OK) if no failure has occurred yet.
 */
int meow_csp_last_ftp_err(void);

/**
 * @brief Return the raw OSAL error code from the most recent module operation failure.
 *
 * @details
 *      - Updated by meow_csp_handler_load on OS_ModuleLoad or
 *            OS_ModuleSymbolLookup failure.
 *      - Returns 0 (OS_SUCCESS) if no failure has occurred yet.
 */
int meow_csp_last_module_err(void);

#endif /* MEOW_INCLUDE_CSP */

#endif /* MEOW_CSP_H */
