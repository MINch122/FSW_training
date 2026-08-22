/**
 * @file esup_session.c
 * @brief Implementation of the ESUP session (transport) state machine.
 *
 * Timing follows the "round every wait up" rule: the module executes on silence
 * and stores results for seconds, so all fixed waits are lower bounds widened
 * for a coarse host clock.
 */
#include "esup_session_priv.h"
#include "esup_utils.h"

#include <string.h>

/*
 * Fixed protocol timing (milliseconds), sized for a non-realtime Linux host with
 * scheduling jitter. Two kinds of value here, both safe to be *large*:
 *
 *  - Floors (CONFIRM_QUIET, COLLISION_GUARD): minimum silence enforced against
 *    the monotonic clock, so jitter can only make them longer, never shorter.
 *  - Give-up windows (ACK_WAIT, FRAME_READ): how long we wait for a reply that
 *    has not arrived. These do NOT add latency in the common case: the reply is
 *    OS-buffered and poll() returns the instant it arrives (~1 ms), so a wide
 *    window is nearly free and only extends the wait when a reply is genuinely
 *    late or missing. They are deliberately well above expected jitter.
 *
 * A late *read* never drops a reply: the OS buffers it and the session consumes
 * the mailbox before its deadline (see esup_session_step ordering).
 */
#define ESUP_ACK_WAIT_MS        50u   /**< Give-up window for the submission reply. */
#define ESUP_CONFIRM_QUIET_MS   10u   /**< Silence floor after ACK (protocol min 2 ms). */
#define ESUP_COLLISION_GUARD_MS 10u   /**< Wait floor after an unanswered frame (min 5 ms). */
#define ESUP_TX_WAIT_MS         100u  /**< Budget to clock out one frame. */
#define ESUP_FRAME_READ_MS      100u  /**< Give-up window for one reply frame. */
#define ESUP_POLL_MIN_MS        20u   /**< Initial Get_Results poll interval. */
#define ESUP_POLL_MAX_MS        1000u /**< Poll interval backoff cap. */

/* Retry and grace budgets. */
#define ESUP_SUBMIT_RETRY_MAX   4u    /**< Resubmits when the container is full. */
#define ESUP_SILENCE_RETRY_MAX  3u    /**< Consecutive silent polls before UNREACHABLE. */
#define ESUP_NCE_GRACE          2u    /**< Early NCE polls tolerated before EXPIRED. */

/**
 * @brief Whether an absolute monotonic deadline has been reached (wrap-safe).
 *
 * @param deadline Absolute deadline.
 * @param now      Current time.
 * @return true if @p deadline is at or before @p now.
 */
static bool deadline_reached(uint32_t deadline, uint32_t now)
{
    return (int32_t)(deadline - now) <= 0;
}

/* ------------------------------------------------------------------------- */
/* Outbound frames (built and sent through the framer)                       */
/* ------------------------------------------------------------------------- */

/**
 * @brief (Re)send a session's command, re-encoded from its retained inputs.
 *
 * @param link      Framer.
 * @param module_id Target unit ID.
 * @param session   Session whose command to send.
 * @return 0 on success; ESUP_SESSION_IO_ERROR with a framer/wire cause on
 *         failure.
 */
static esup_ret_t send_command(esup_framer_t* link, uint16_t module_id,
                        const esup_session_t* session)
{
    esup_frame_t f;

    f.module_id = module_id;
    f.cmd_status = ESUP_STATUS_NONE;
    f.command = session->command;
    f.type = session->type;
    f.data = session->req_data;
    f.data_len = session->req_len;
    esup_ret_t fret = esup_framer_send(link, &f, esup_framer_now(link) + ESUP_TX_WAIT_MS);

    if (fret != ESUP_OK) {
        ESUP_LOGE("command send failed (cmd=0x%04X type=0x%04X)",
                  session->command, session->type);
        return ESUP_SESSION_IO_ERROR | fret;
    }
    return ESUP_OK;
}

/**
 * @brief Send the Get_Results query for a session.
 *
 * The query names the target command in the Type field and the target type in
 * the two-byte Data field (empty when the original command had no type).
 *
 * @param link      Framer.
 * @param module_id Target unit ID.
 * @param session   Session to poll.
 * @return 0 on success; ESUP_SESSION_IO_ERROR on failure.
 */
static esup_ret_t send_get_results(esup_framer_t* link, uint16_t module_id,
                            const esup_session_t* session)
{
    uint8_t sel[2];
    esup_frame_t f;

    sel[0] = (uint8_t)(session->type & 0xFFu);
    sel[1] = (uint8_t)((session->type >> 8) & 0xFFu);

    f.module_id = module_id;
    f.cmd_status = ESUP_STATUS_NONE;
    f.command = ESUP_CMD_GET_RESULTS;
    f.type = session->command;
    f.data = (session->type != ESUP_TYPE_NONE) ? sel : NULL;
    f.data_len = (session->type != ESUP_TYPE_NONE) ? 2u : 0u;
    esup_ret_t fret = esup_framer_send(link, &f, esup_framer_now(link) + ESUP_TX_WAIT_MS);

    if (fret != ESUP_OK) {
        ESUP_LOGE("poll send failed (cmd=0x%04X type=0x%04X)",
                  session->command, session->type);
        return ESUP_SESSION_IO_ERROR | fret;
    }
    return ESUP_OK;
}

/**
 * @brief Send the ACK confirming receipt of a result, freeing the device cell.
 *
 * @param link      Framer.
 * @param module_id Target unit ID.
 * @param session   Session whose result is acknowledged.
 * @return 0 on success; a framer/wire cause on failure. The caller adds the
 *         engine-level ESUP_SESSION_ACK_FAILED outcome.
 */
static esup_ret_t send_result_ack(esup_framer_t* link, uint16_t module_id,
                           const esup_session_t* session)
{
    esup_frame_t f;

    f.module_id = module_id;
    f.cmd_status = ESUP_STATUS_ACK;
    f.command = session->command;
    f.type = session->type;
    f.data = NULL;
    f.data_len = 0;
    esup_ret_t fret = esup_framer_send(link, &f, esup_framer_now(link) + ESUP_TX_WAIT_MS);

    if (fret != ESUP_OK) {
        ESUP_LOGE("result-ack send failed (cmd=0x%04X type=0x%04X)",
                  session->command, session->type);
        return fret;
    }
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* Lifecycle and mailbox                                                     */
/* ------------------------------------------------------------------------- */

/**
 * @brief Terminate a session with a result and free its slot.
 *
 * The single exit point for every transaction, so no path can leak a slot.
 *
 * @param session Session to terminate.
 * @param res     Terminal result.
 */
static void terminate(esup_session_t* session, esup_ret_t res)
{
    if (res == ESUP_OK)
        ESUP_LOGI("cmd=0x%04X type=0x%04X ok", session->command, session->type);
    else if (ESUP_RET_ENGINE(res) == ESUP_SESSION_IO_ERROR)
        ESUP_LOGE("cmd=0x%04X type=0x%04X i/o error (ret 0x%08X)",
                  session->command, session->type, (unsigned)res);
    else
        ESUP_LOGW("cmd=0x%04X type=0x%04X terminated (ret 0x%08X)",
                  session->command, session->type, (unsigned)res);
    session->res = res;
    session->state = ESUP_SESSION_DONE;
    session->rx_pending = false;
    /* The slot stays in_use (retained) until esup_session_reap frees it, so a
     * still-held handle can never observe a different, recycled transaction. */
}

void esup_session_init(esup_session_t* session, const void* req_data,
                      uint16_t req_len, esup_result_t* result, uint32_t now,
                      uint32_t timeout_ms)
{
    session->req_data = (req_len > 0) ? req_data : NULL;
    session->req_len = req_len;
    session->result = result;
    session->deadline_ms = now + timeout_ms;
    session->next_action_ms = now;
    session->poll_interval_ms = ESUP_POLL_MIN_MS;
    session->submit_attempts = 0;
    session->silence = 0;
    session->nce = 0;
    session->saw_busy = false;
    session->saw_reply = false;
    session->rx_pending = false;
    session->rx_complete = true;
    session->res = ESUP_SESSION_ABORTED;
    session->state = ESUP_SESSION_SEND_CMD;
}

/* ------------------------------------------------------------------------- */
/* Public accessors (opaque-handle observation)                              */
/* ------------------------------------------------------------------------- */

bool esup_session_is_done(const esup_session_t* session)
{
    return session != NULL && session->state == ESUP_SESSION_DONE;
}

esup_ret_t esup_session_result(const esup_session_t* session)
{
    if (session == NULL)
        return ESUP_SESSION_ABORTED;
    if (session->state != ESUP_SESSION_DONE)
        return ESUP_SESSION_PENDING;
    return session->res;
}

void esup_session_selector(const esup_session_t* session, uint16_t* command,
                           uint16_t* type)
{
    if (session == NULL)
        return;
    if (command != NULL)
        *command = session->command;
    if (type != NULL)
        *type = session->type;
}

bool esup_session_is_live(const esup_session_t* session)
{
    return session->in_use && session->state != ESUP_SESSION_DONE
        && session->state != ESUP_SESSION_FREE;
}

void esup_session_cancel(esup_session_t* session)
{
    if (esup_session_is_live(session))
        terminate(session, ESUP_SESSION_UNREACHABLE);
}

void esup_session_reap(esup_session_t* session)
{
    if (session->state != ESUP_SESSION_DONE)
        return;
    session->in_use = false;
    session->state = ESUP_SESSION_FREE;
}

bool esup_session_awaiting(const esup_session_t* session, uint16_t command,
                           uint16_t type)
{
    if (!session->in_use || session->rx_pending)
        return false;
    if (session->state != ESUP_SESSION_AWAIT_ACK
        && session->state != ESUP_SESSION_AWAIT_RESULT)
        return false;
    return session->command == command && session->type == type;
}

void esup_session_deposit(esup_session_t* session, const esup_frame_t* frame)
{
    const uint8_t* bytes = frame->data;

    session->rx_status = frame->cmd_status;
    session->rx_data_len = frame->data_len;
    session->rx_exec = (frame->data_len > 0) ? bytes[0] : (uint8_t)ESUP_EXEC_OK;
    session->rx_complete = true;

    /* Only a successful Get_Results envelope carries a command result. Copy its
     * complete raw Data field when the caller supplied storage. A NULL data
     * pointer is an explicit discard; a non-NULL undersized buffer is not. */
    if (session->state == ESUP_SESSION_AWAIT_RESULT
        && frame->cmd_status == ESUP_STATUS_NONE && session->result != NULL) {
        esup_result_t* r = session->result;
        uint16_t n = frame->data_len;

        r->command = frame->command;
        r->type = frame->type;
        r->data_len = frame->data_len;
        if (r->data != NULL && n > r->data_cap) {
            session->rx_complete = false;
            n = r->data_cap;
        }
        if (r->data != NULL && n > 0)
            memcpy(r->data, bytes, n);
    }
    session->rx_pending = true;
}

/* ------------------------------------------------------------------------- */
/* State machine                                                             */
/* ------------------------------------------------------------------------- */

/**
 * @brief Advance the poll backoff interval after a BUSY.
 *
 * @param session Session to back off.
 */
static void backoff_poll(esup_session_t* session)
{
    session->poll_interval_ms *= 2u;
    if (session->poll_interval_ms > ESUP_POLL_MAX_MS)
        session->poll_interval_ms = ESUP_POLL_MAX_MS;
}

/**
 * @brief Handle the submission reply consumed in AWAIT_ACK.
 *
 * @param session Session.
 * @param now     Current time.
 */
static void step_submit_reply(esup_session_t* session, uint32_t now)
{
    session->rx_pending = false;
    session->saw_reply = true;

    switch (session->rx_status) {
    case ESUP_STATUS_ACK:
    case ESUP_STATUS_BUSY:
        /* ACK = accepted. BUSY at submission is ambiguous (the manual says "that
         * command is currently executing"), so we take the NON-DESTRUCTIVE path:
         * observe the quiet window and poll, rather than resend and risk a
         * double-submit that would desync the container. If BUSY meant "not
         * accepted", polling simply yields NCE -> EXPIRED. VERIFY against a
         * capture. */
        session->state = ESUP_SESSION_QUIET;
        session->next_action_ms = now + ESUP_CONFIRM_QUIET_MS;
        return;

    case ESUP_STATUS_NOT_ACK:
        terminate(session, ESUP_SESSION_REJECTED);
        return;

    case ESUP_STATUS_STACK_FULL:
    case ESUP_STATUS_TEMP_NOT_ACCEPTED:
        /* Definitely not accepted (no free cell). Resubmitting is safe because
         * the command never entered the container; bound the retries. */
        session->submit_attempts++;
        if (session->submit_attempts > ESUP_SUBMIT_RETRY_MAX) {
            terminate(session, ESUP_SESSION_FULL);
            return;
        }
        session->state = ESUP_SESSION_SEND_CMD;
        session->next_action_ms = now + ESUP_COLLISION_GUARD_MS;
        return;

    default:
        /* Any other status at submission is unmodelled: do not assume the command
         * was accepted. Report a host/device desync rather than guessing. */
        ESUP_LOGW("unexpected submission status 0x%04X", session->rx_status);
        terminate(session, ESUP_SESSION_DESYNC);
        return;
    }
}

/**
 * @brief Handle the poll reply consumed in AWAIT_RESULT.
 *
 * @param session Session.
 * @param now     Current time.
 */
static void step_poll_reply(esup_session_t* session, uint32_t now)
{
    session->rx_pending = false;
    session->saw_reply = true;
    session->silence = 0;

    if (session->rx_status == ESUP_STATUS_BUSY) {
        session->saw_busy = true;
        backoff_poll(session);
        session->state = ESUP_SESSION_SEND_POLL;
        session->next_action_ms = now + session->poll_interval_ms;
        return;
    }
    if (session->rx_status == ESUP_STATUS_NCE) {
        if (session->saw_busy) {
            terminate(session, ESUP_SESSION_EXPIRED);
            return;
        }
        session->nce++;
        if (session->nce >= ESUP_NCE_GRACE) {
            terminate(session, ESUP_SESSION_EXPIRED);
            return;
        }
        session->state = ESUP_SESSION_SEND_POLL;
        session->next_action_ms = now + ESUP_POLL_MIN_MS;
        return;
    }
    if (session->rx_status == ESUP_STATUS_NONE) {
        if (session->rx_data_len == 0u) {
            ESUP_LOGW("result has no execution-status byte");
            terminate(session, ESUP_SESSION_DESYNC);
            return;
        }
        if (!session->rx_complete) {
            ESUP_LOGW("result length %u exceeds caller capacity", session->rx_data_len);
            terminate(session, ESUP_SESSION_RESULT_TOO_LARGE
                               | (esup_ret_t)session->rx_exec);
            return;
        }
        /* Result received and deposited into the caller's buffer. Do NOT declare
         * success yet: confirm receipt with a result-ACK first (ESUP_SESSION_
         * ACK_RESULT), so a failed ACK is reported instead of a false success. */
        session->state = ESUP_SESSION_ACK_RESULT;
        session->next_action_ms = now;
        return;
    }
    /* Anything else (e.g. a late submission ACK that leaked into the poll phase,
     * or a duplicate) is a stray. Drop it and keep waiting for the real result
     * rather than killing a live transaction. */
}

/**
 * @brief Send the result-ACK and reach a terminal outcome (ESUP_SESSION_ACK_RESULT).
 *
 * @param session   Session.
 * @param link      Framer.
 * @param module_id Target unit ID.
 */
static void step_ack_result(esup_session_t* session, esup_framer_t* link,
                            uint16_t module_id)
{
    esup_ret_t cause = send_result_ack(link, module_id, session);
    /* The module's execution-status byte rides in the device region (byte 0);
     * it is 0 on success, so ESUP_OK stays 0. */
    esup_ret_t exec = (esup_ret_t)session->rx_exec;

    if (cause != ESUP_OK) {
        /* ACK_FAILED is the sole engine-field outcome; cause contains only the
         * disjoint framer/wire fields and exec occupies the device-status byte. */
        terminate(session, ESUP_SESSION_ACK_FAILED | cause | exec);
        return;
    }
    if (session->rx_exec == ESUP_EXEC_OK)
        terminate(session, ESUP_OK | exec);
    else
        terminate(session, ESUP_SESSION_EXEC_ERROR | exec);
}

void esup_session_step(esup_session_t* session, esup_framer_t* link,
                       uint16_t module_id, uint32_t now)
{
    if (session->state == ESUP_SESSION_FREE || session->state == ESUP_SESSION_DONE)
        return;

    /* A reply that already arrived wins over a coincident deadline: consume the
     * mailbox before enforcing the deadline, so a result received in time is
     * never discarded by a deadline that expired in the same step. */
    if (session->rx_pending) {
        if (session->state == ESUP_SESSION_AWAIT_ACK)
            step_submit_reply(session, now);
        else if (session->state == ESUP_SESSION_AWAIT_RESULT)
            step_poll_reply(session, now);
        else
            session->rx_pending = false; /* frame for a non-awaiting state: drop */
        return;
    }

    /* Once the result is in hand, confirm it regardless of the overall deadline:
     * the data is already captured, so the ACK (and its outcome) must not be
     * pre-empted by a deadline that expires in the same window. */
    if (session->state == ESUP_SESSION_ACK_RESULT) {
        step_ack_result(session, link, module_id);
        return;
    }

    if (deadline_reached(session->deadline_ms, now)) {
        if (session->saw_busy)
            terminate(session, ESUP_SESSION_BUSY_TIMEOUT);
        else if (session->saw_reply)
            terminate(session, ESUP_SESSION_DESYNC);
        else
            terminate(session, ESUP_SESSION_UNREACHABLE);
        return;
    }

    switch (session->state) {
    case ESUP_SESSION_SEND_CMD:
        if (!deadline_reached(session->next_action_ms, now))
            return;
        esup_ret_t sret = send_command(link, module_id, session);

        if (sret != ESUP_OK) {
            terminate(session, sret); /* carries the framer/wire cause */
            return;
        }
        session->state = ESUP_SESSION_AWAIT_ACK;
        session->next_action_ms = now + ESUP_ACK_WAIT_MS;
        return;

    case ESUP_SESSION_AWAIT_ACK:
        /* rx_pending handled above; here only the no-ACK timeout remains. */
        if (deadline_reached(session->next_action_ms, now)) {
            /* No ACK seen; best-effort, let polling disambiguate. */
            session->state = ESUP_SESSION_QUIET;
            session->next_action_ms = now + ESUP_CONFIRM_QUIET_MS;
        }
        return;

    case ESUP_SESSION_QUIET:
        if (deadline_reached(session->next_action_ms, now)) {
            session->state = ESUP_SESSION_SEND_POLL;
            session->next_action_ms = now;
            session->poll_interval_ms = ESUP_POLL_MIN_MS;
        }
        return;

    case ESUP_SESSION_SEND_POLL:
        if (!deadline_reached(session->next_action_ms, now))
            return;
        esup_ret_t pret = send_get_results(link, module_id, session);

        if (pret != ESUP_OK) {
            terminate(session, pret); /* carries the framer/wire cause */
            return;
        }
        session->state = ESUP_SESSION_AWAIT_RESULT;
        session->next_action_ms = now + ESUP_FRAME_READ_MS;
        return;

    case ESUP_SESSION_AWAIT_RESULT:
        /* rx_pending handled above; here only the reply-read timeout remains. */
        if (deadline_reached(session->next_action_ms, now)) {
            session->silence++;
            if (session->silence >= ESUP_SILENCE_RETRY_MAX)
                terminate(session, ESUP_SESSION_UNREACHABLE);
            else {
                session->state = ESUP_SESSION_SEND_POLL;
                session->next_action_ms = now + ESUP_COLLISION_GUARD_MS;
            }
        }
        return;

    default:
        return;
    }
}
