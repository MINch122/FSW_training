/**
 * @file esup_session.h
 * @brief ESUP transport layer (public): the transaction result type and an
 *        opaque handle to one in-flight transaction.
 *
 * A session owns the lifecycle of a single ESUP transaction: issue the
 * command, read the ACK, observe the quiet window, poll Get_Results, and 
 * deliver the result.
 * Sessions are statically allocated inside the engine's pool; a caller
 * never allocates one and only ever holds an opaque handle returned by
 * esup_start_session. The concrete struct and the reply-driven FSM are
 * private to the engine (see esup_session_priv.h).
 */
#ifndef ESUP_SESSION_H
#define ESUP_SESSION_H

#include <stdbool.h>
#include <stdint.h>

#include "esup_ret.h"

/**
 * @brief Session (transport) return codes (byte 3 of esup_ret_t).
 *
 * This layer's field is the transaction termination outcome: ESUP_OK, or one of
 * these naming a distinct fault scope so a ground operator can act on it. The
 * driver reports these and stops; it does not attempt recovery. A lower-layer
 * cause (framer / wire) may be OR'd into the value alongside these.
 */
enum {
    ESUP_SESSION_EXEC_ERROR   = 0x01000000, /**< Result received, execution status non-zero. */
    ESUP_SESSION_REJECTED     = 0x02000000, /**< Module returned NOT_ACK at submission. */
    ESUP_SESSION_EXPIRED      = 0x03000000, /**< No command for execution (result lost / never enqueued). */
    ESUP_SESSION_BUSY_TIMEOUT = 0x04000000, /**< Bounded out while the module stayed BUSY. */
    ESUP_SESSION_UNREACHABLE  = 0x05000000, /**< No reply at all; link or device fault. */
    ESUP_SESSION_IO_ERROR     = 0x06000000, /**< Host transport fault. */
    ESUP_SESSION_ABORTED      = 0x07000000, /**< Bad request or engine not ready. */
    ESUP_SESSION_DESYNC       = 0x08000000, /**< Reply irreconcilable with session state. */
    ESUP_SESSION_PENDING      = 0x09000000, /**< Not terminal: esup_wait timed out while
                                             the transaction was still running. */
    ESUP_SESSION_FULL         = 0x0A000000, /**< No free container cell; retry later. */
    ESUP_SESSION_DUP          = 0x0B000000, /**< A session with this selector is already active. */
    ESUP_SESSION_ACK_FAILED   = 0x0C000000, /**< Complete result received, but its ACK could not
                                                 be sent; device cell disposition is unknown. */
    ESUP_SESSION_RESULT_TOO_LARGE = 0x0D000000 /**< Result exceeded the supplied buffer and was
                                                    deliberately not ACKed. */
};

/**
 * @brief Captured result of a transaction.
 *
 * The payload buffer is caller-owned: set @ref data and @ref data_cap before the
 * call (or leave @ref data NULL to discard the payload). After completion the
 * engine has filled @ref command, @ref type, and @ref data_len (the device's
 * reported length) and copied the raw Data field into @ref data. If a non-NULL
 * buffer is too small, the session returns ESUP_SESSION_RESULT_TOO_LARGE and does
 * not ACK the result, so the device retains it until its normal TTL. A NULL data
 * pointer explicitly discards the raw Data field and does not cause this error.
 *
 * The raw Data field includes command-execution status at byte 0. The same byte
 * is mirrored in the transaction's esup_ret_t and extracted with
 * ESUP_RET_EXEC_STATUS(); its command-specific meaning belongs to the device
 * driver. Bytes 1..N are the command-specific payload.
 */
typedef struct {
    uint16_t command;  /**< Out: echoed command selector. */
    uint16_t type;     /**< Out: echoed type selector. */
    uint8_t* data;     /**< In: caller's payload buffer, or NULL to discard. */
    uint16_t data_cap; /**< In: capacity of @ref data in bytes. */
    uint16_t data_len; /**< Out: Data field length reported by the device. */
} esup_result_t;

/**
 * @brief Opaque handle to one in-flight transaction.
 *
 * Returned by esup_start_session and owned by the engine's pool; the caller only
 * observes it through the accessors below and esup_wait.
 */
typedef struct esup_session esup_session_t;

/**
 * @brief Whether a session has reached its terminal state.
 *
 * @param session Session handle.
 * @return true once the transaction is complete (its result is final).
 */
bool esup_session_is_done(const esup_session_t* session);

/**
 * @brief The session's result code.
 *
 * @param session Session handle.
 * @return The terminal esup_ret_t once done; ESUP_SESSION_PENDING while it still runs.
 */
esup_ret_t esup_session_result(const esup_session_t* session);

/**
 * @brief Read a session's {command, type} selector.
 *
 * @param session Session handle.
 * @param command Out: command selector (ignored if NULL).
 * @param type    Out: type selector (ignored if NULL).
 */
void esup_session_selector(const esup_session_t* session, uint16_t* command,
                           uint16_t* type);

#endif /* ESUP_SESSION_H */
