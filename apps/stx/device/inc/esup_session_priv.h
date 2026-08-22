/**
 * @file esup_session_priv.h
 * @brief ESUP transport layer (private): the concrete session struct and the
 *        reply-driven FSM the engine drives.
 *
 * Engine-internal. Included by esup_session.c (which implements the FSM) and by
 * esup.c (which embeds the session pool by value and steps it). Device drivers
 * and applications include only esup_session.h and use the opaque handle.
 *
 * Timing follows the "round every wait up" rule: the module executes on silence
 * and stores results for seconds, so being late is safe and being early corrupts
 * the transaction.
 */
#ifndef ESUP_SESSION_PRIV_H
#define ESUP_SESSION_PRIV_H

#include <stdbool.h>
#include <stdint.h>

#include "esup_framer.h"
#include "esup_proto.h"
#include "esup_session.h"

/**
 * @brief Session state (the reply-driven FSM).
 */
typedef enum {
    ESUP_SESSION_FREE = 0,       /**< Slot unused. */
    ESUP_SESSION_SEND_CMD,       /**< Time to (re)send the command. */
    ESUP_SESSION_AWAIT_ACK,      /**< Command sent; awaiting the submission reply. */
    ESUP_SESSION_QUIET,          /**< Confirm window: stay silent before executing. */
    ESUP_SESSION_SEND_POLL,      /**< Time to send Get_Results. */
    ESUP_SESSION_AWAIT_RESULT,   /**< Get_Results sent; awaiting the reply. */
    ESUP_SESSION_ACK_RESULT,     /**< Result in hand; confirm it with an ACK before DONE. */
    ESUP_SESSION_DONE            /**< Terminated; see @ref esup_session::res. */
} esup_session_state_t;

/**
 * @brief One device-container transaction. Fields are transport-private; callers
 *        observe only through the esup_session.h accessors.
 */
struct esup_session {
    bool                 in_use;        /**< The slot is allocated. */
    uint8_t              state;         /**< Current FSM state (esup_session_state_t). */
    esup_ret_t           res;           /**< Terminal result once DONE. */

    uint16_t             command;       /**< Command. */
    uint16_t             type;          /**< Type. */
    const void*          req_data;      /**< Caller's request payload (borrowed), or NULL. */
    uint16_t             req_len;       /**< Request payload length. */
    esup_result_t*       result;        /**< Caller's result descriptor (borrowed), or NULL. */

    uint32_t             deadline_ms;   /**< Overall transaction deadline. */
    uint32_t             next_action_ms;/**< When to next act (send due, or await timeout). */
    uint32_t             poll_interval_ms; /**< Current Get_Results backoff interval. */
    uint16_t             submit_attempts;  /**< Resubmits used against a full container. */

    uint16_t             silence;       /**< Consecutive unanswered polls. */
    uint16_t             nce;           /**< Early NCE replies tolerated. */
    bool                 saw_busy;      /**< The module reported BUSY at least once. */
    bool                 saw_reply;     /**< Any reply was received. */

    bool                 rx_pending;    /**< A reply is waiting in the mailbox. */
    bool                 rx_complete;   /**< Result fitted, or payload discard was explicit. */
    uint16_t             rx_status;     /**< Mailbox: reply Command Status. */
    uint16_t             rx_data_len;   /**< Mailbox: reply Data length. */
    uint8_t              rx_exec;       /**< Mailbox: reply execution status (Data byte 0). */
};

/**
 * @brief Arm a claimed session slot for a new transaction.
 *
 * @param session    Session slot (with in_use, command, type already set by the pool).
 * @param req_data   Request payload (borrowed), or NULL when @p req_len is 0.
 * @param req_len    Request payload length.
 * @param result     Result descriptor (borrowed), or NULL.
 * @param now        Current monotonic time.
 * @param timeout_ms Overall transaction budget in milliseconds.
 */
void esup_session_init(esup_session_t* session, const void* req_data,
                      uint16_t req_len, esup_result_t* result, uint32_t now,
                      uint32_t timeout_ms);

/**
 * @brief Whether a session is currently awaiting a reply for a selector.
 *
 * The routing predicate: true only for a receptive session with an empty
 * mailbox, which drops stale/duplicate frames and prevents overwrite.
 *
 * @param session Session to test.
 * @param command Frame command.
 * @param type    Frame type.
 * @return true if this session owns and awaits a reply for the selector.
 */
bool esup_session_awaiting(const esup_session_t* session, uint16_t command,
                           uint16_t type);

/**
 * @brief Deposit a decoded reply frame into a session's mailbox.
 *
 * Copies the scalar header, and for a result frame the payload into the caller's
 * result buffer (up to its capacity). Must run before the next framer call
 * invalidates the frame's data pointer.
 *
 * @param session Owning session.
 * @param frame   Decoded frame.
 */
void esup_session_deposit(esup_session_t* session, const esup_frame_t* frame);

/**
 * @brief Advance a session by one non-blocking step (sends via @p link).
 *
 * @param session   Session to advance.
 * @param link      Framer used for outbound frames.
 * @param module_id Target unit ID for outbound frames.
 * @param now       Current monotonic time.
 */
void esup_session_step(esup_session_t* session, esup_framer_t* link,
                       uint16_t module_id, uint32_t now);

/**
 * @brief Whether a session slot holds work the driver must still advance.
 *
 * A terminated (DONE) session is retained in its slot until it is reaped, so it
 * is occupied but NOT live: the driver must skip it and must not let it influence
 * the wake schedule.
 *
 * @param session Session slot.
 * @return true if the session is allocated and not yet terminated.
 */
bool esup_session_is_live(const esup_session_t* session);

/**
 * @brief Force a still-running session to a terminal UNREACHABLE outcome.
 *
 * Used by the engine when a blocking caller gives up: the session must stop
 * referencing the caller's borrowed buffers. A no-op on an already terminal or
 * free slot.
 *
 * @param session Session to cancel.
 */
void esup_session_cancel(esup_session_t* session);

/**
 * @brief Free a terminated session's slot for reuse. A no-op unless DONE.
 *
 * @param session Session to reap.
 */
void esup_session_reap(esup_session_t* session);

#endif /* ESUP_SESSION_PRIV_H */
