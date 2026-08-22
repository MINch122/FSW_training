/**
 * @file esup.h
 * @brief ESUP engine: session pool, reply routing, and a simple driver.
 *
 * Layering: wire (bytes + time) -> framer (frames, both directions) -> session
 * (one transaction FSM + mailbox) -> engine.
 *
 * Model:
 *   - esup_start_session issues a command: it sends the frame immediately and
 *     hands back an opaque session handle with the FSM armed for the reply. It
 *     never waits for anything.
 *   - The driver (a plain repeated tick) reads the wire, routes replies to the
 *     owning sessions, and advances their timers (polls, retries, deadlines).
 *     Run it with esup_start (spawns the pthread), or esup_run from a thread you
 *     own, or call esup_tick from your own main loop.
 *   - esup_wait is a timed wait on one session's completion.
 *
 * The engine is an opaque, heap-allocated object (esup_engine_create); sessions
 * are opaque handles into its internal pool. A caller never allocates either.
 *
 * Typical use:
 *   esup_engine_t* eng = esup_engine_create(&wire, 0x1023);
 *   esup_start(eng);                                     // once
 *   esup_session_t* s;
 *   esup_start_session(eng, cmd, type, ..., &s);          // fires immediately
 *   int rc = esup_wait(eng, s, 2000);                     // result or PENDING
 *
 * Thread safety: esup_start_session and esup_wait may be called from any thread.
 * One mutex guards the session pool and the transmit path; it is never held while
 * blocked reading the wire. Only one driver may run at a time.
 *
 * Lifetime contract: the request payload and the result buffer passed to
 * esup_start_session must stay valid until the session is done.
 */
#ifndef ESUP_H
#define ESUP_H

#include <stdbool.h>
#include <stdint.h>

#include "esup_proto.h"
#include "esup_session.h"
#include "esup_wire_linux.h"
#include "esup_config.h"

/** Opaque transaction engine bound to one module over one link. */
typedef struct esup_engine esup_engine_t;

/**
 * @brief Create an engine over an opened wire (heap-allocated).
 *
 * @param wire      An opened transport handle; copied into the engine's framer.
 * @param module_id Target unit ID (the last four hex digits of the serial).
 * @return A new engine, or NULL on allocation or initialisation failure. Release
 *         it with esup_engine_destroy.
 */
esup_engine_t* esup_engine_create(const esup_wire_t* wire, uint16_t module_id);

/**
 * @brief Stop any driver, release engine resources, and free the engine.
 *
 * Does not close the wire (the caller owns it). Safe on NULL.
 *
 * @param engine Engine to destroy.
 */
void esup_engine_destroy(esup_engine_t* engine);

/**
 * @brief Host-side reset: clear the reassembler and drop all sessions.
 *
 * Purely local. Must not run while a driver is active (esup_stop first);
 * refused with ESUP_SESSION_ABORTED then.
 *
 * @param engine Engine to reset.
 * @return 0 on success; a nonzero esup_ret_t on failure.
 */
esup_ret_t esup_engine_reset(esup_engine_t* engine);

/**
 * @brief Issue a command: send its frame now and arm the session for the reply.
 *
 * Allocates a session (selector uniqueness enforced), transmits the command
 * frame, and writes the handle to @p out_session. Up to ESUP_SESSION_MAX commands
 * may be in flight. Collect with esup_wait or via the esup_session.h accessors. If
 * the transmit itself fails the session is returned already done with IO_ERROR.
 *
 * @param engine      Engine.
 * @param command     Command code (ESUP_CMD_* or device-specific).
 * @param type        Type code (device-specific, or ESUP_TYPE_NONE).
 * @param req_data    Request payload (borrowed), or NULL when @p req_len is 0.
 * @param req_len     Request payload length in bytes.
 * @param result      Result descriptor (borrowed), or NULL to discard the payload.
 * @param timeout_ms  Overall transaction budget in milliseconds.
 * @param out_session Out: the session handle on success, NULL on failure.
 * @return 0 (ESUP_OK) on success; a nonzero esup_ret_t on failure.
 */
esup_ret_t esup_start_session(esup_engine_t* engine, uint16_t command, uint16_t type,
                       const void* req_data, uint16_t req_len,
                       esup_result_t* result, uint32_t timeout_ms,
                       esup_session_t** out_session);

/**
 * @brief Run one transaction and block until it completes (start + wait).
 *
 * The normal, blocking way to talk to the device: it starts a session, waits for
 * the driver to reap the result, and returns the outcome. A driver must be
 * running (esup_start / esup_run / a manual esup_tick loop) for progress to
 * happen. For overlapping transactions, use esup_start_session + esup_wait
 * directly instead.
 *
 * The wait runs slightly past @p timeout_ms so a session-side timeout surfaces as
 * its real result rather than PENDING. A refused start (no free slot, duplicate
 * selector, bad argument) is reported as ESUP_SESSION_ABORTED.
 *
 * @param engine     Engine.
 * @param command    Command code (ESUP_CMD_* or device-specific).
 * @param type       Type code (device-specific, or ESUP_TYPE_NONE).
 * @param req_data   Request payload (borrowed), or NULL when @p req_len is 0.
 * @param req_len    Request payload length in bytes.
 * @param result     Result descriptor (borrowed), or NULL to discard the payload.
 * @param timeout_ms Overall transaction budget in milliseconds.
 * @return 0 (ESUP_OK) on success; a nonzero esup_ret_t otherwise.
 */
esup_ret_t esup_transact(esup_engine_t* engine, uint16_t command, uint16_t type,
                  const void* req_data, uint16_t req_len, esup_result_t* result,
                  uint32_t timeout_ms);

/**
 * @brief Convenience: run a generic ESUP GET of a parameter Type (blocking).
 *
 * @param engine     Engine.
 * @param type       Parameter Type code.
 * @param result     Result descriptor (borrowed), or NULL.
 * @param timeout_ms Overall transaction budget in milliseconds.
 * @return 0 on success; a nonzero esup_ret_t otherwise.
 */
esup_ret_t esup_get(esup_engine_t* engine, uint16_t type, esup_result_t* result,
             uint32_t timeout_ms);

/**
 * @brief Convenience: run a generic ESUP SET of a parameter Type (blocking).
 *
 * @param engine     Engine.
 * @param type       Parameter Type code.
 * @param data       Payload to write (borrowed), or NULL when @p len is 0.
 * @param len        Payload length in bytes.
 * @param result     Result descriptor (borrowed), or NULL.
 * @param timeout_ms Overall transaction budget in milliseconds.
 * @return 0 on success; a nonzero esup_ret_t otherwise.
 */
esup_ret_t esup_set(esup_engine_t* engine, uint16_t type, const void* data,
             uint16_t len, esup_result_t* result, uint32_t timeout_ms);

/**
 * @brief Wait (with a timeout) for one transaction to complete.
 *
 * A timed wait on the engine's completion signal: returns as soon as the driver
 * marks @p session done, or ESUP_SESSION_PENDING when the timeout expires first
 * (the transaction keeps running; wait again or observe it). A driver must be
 * running for progress to happen (an already-done session returns immediately
 * regardless).
 *
 * For the start_session + wait path only: a completed session is RETAINED in its
 * slot (so a held handle can never observe a recycled transaction) until you call
 * esup_reap. The lifetime contract holds until you reap: the request payload and
 * result buffer must stay valid, and if you drop them you must ensure the session
 * is terminal first (esup_wait returned a non-PENDING code). esup_transact does
 * all of this for you.
 *
 * @param engine     Engine.
 * @param session    Handle from esup_start_session, or NULL (returns ABORTED).
 * @param timeout_ms Maximum time to wait, in milliseconds.
 * @return The terminal esup_ret_t, ESUP_SESSION_PENDING on timeout, or
 *         ESUP_SESSION_ABORTED on a null argument.
 */
esup_ret_t esup_wait(esup_engine_t* engine, esup_session_t* session,
              uint32_t timeout_ms);

/**
 * @brief Release a completed session's slot so it can be reused.
 *
 * Required for the start_session + wait path once you are done observing a
 * terminal session; the slot stays occupied until then. A no-op if the session
 * is not yet terminal or is NULL. esup_transact reaps internally, so its callers
 * never call this.
 *
 * @param engine  Engine.
 * @param session Session to reap.
 */
void esup_reap(esup_engine_t* engine, esup_session_t* session);

/**
 * @brief One driver pass: read and route replies, then advance session timers.
 *
 * For integrating the engine into an existing main loop. Blocks at most until
 * the soonest scheduled session action; returns immediately when nothing is
 * active. Exactly one context may drive the engine — do not call this while
 * esup_start / esup_run is active.
 *
 * @param engine Engine.
 * @return The number of active sessions (>= 0); -1 on a null argument. This
 *         returns a count, not an esup_ret_t code.
 */
int esup_tick(esup_engine_t* engine);

/**
 * @brief The driver loop: tick while there is work, sleep while idle, exit on
 *        esup_stop. This is the thread body used by esup_start; call it directly
 *        to drive the engine from a thread you own.
 *
 * @param engine Engine.
 * @return 0 on a clean stop; ESUP_SESSION_ABORTED if a driver already runs.
 */
esup_ret_t esup_run(esup_engine_t* engine);

/**
 * @brief Spawn the driver thread (pthread wrapper around esup_run).
 *
 * @param engine Engine.
 * @return 0 on success; ESUP_SESSION_ABORTED if a driver already runs;
 *         ESUP_SESSION_IO_ERROR if thread creation failed.
 */
esup_ret_t esup_start(esup_engine_t* engine);

/**
 * @brief Stop the driver loop and, if esup_start created it, join the thread.
 *
 * In-flight sessions are left as they are and resume under the next driver.
 *
 * @param engine Engine.
 * @return 0 on success; a nonzero esup_ret_t on a null argument.
 */
esup_ret_t esup_stop(esup_engine_t* engine);

/**
 * @brief Read the engine's target module ID.
 *
 * @param engine Engine.
 * @return The module ID, or 0 if @p engine is NULL.
 */
uint16_t esup_engine_module_id(const esup_engine_t* engine);

/**
 * @brief Set the engine's target module ID.
 *
 * Intended for between transactions; changing it while sessions are in flight
 * will misroute their replies.
 *
 * @param engine    Engine.
 * @param module_id New target unit ID.
 */
void esup_engine_set_module_id(esup_engine_t* engine, uint16_t module_id);

/**
 * @brief Whether a driver loop is currently active.
 *
 * @param engine Engine.
 * @return true if esup_run/esup_start is driving the engine.
 */
bool esup_engine_is_running(const esup_engine_t* engine);

#endif /* ESUP_H */
