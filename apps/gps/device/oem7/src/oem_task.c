/**
 * @file oem_task.c
 * @brief OEM7 log receiving tasks.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include "oem_task.h"
#include "oem_io.h"
#include "msg/oem_msg_common.h"
#include "oem_utils.h"

#include <string.h>
#include <time.h>
#if OEM_DEBUG
#include <stdio.h> /* Response print */
#endif

typedef struct {
    union {
        uint8_t bytes[OEM_TASK_STATE_MACHINE_BUF_SIZE];
        oem_binary_header_t header;
    } buffer;
    size_t  need;
    size_t  have;
    uint8_t state;
    uint8_t target;
} oem_task_state_machine_t;

static oem_task_state_machine_t state_machines[OEM_IO_INTERFACES];

/**
 * @brief Pass a Log Message to the log layer for processing.
 *        This is intended to be called by the task layer only. Do not call
 *        this in a user context.
 */
int oem_log_do_handle(void* msg, oem_task_context_t* ctx);

static int process_reply(void* message,
                         oem_task_context_t* ctx)
{
    int ret = OEM_OK;

    if (!ctx)
        return OEM_ERR_NULL;

    if (ctx->is_response == false) {
        /**
         * This is a log message; call the handler.
         */
        return oem_log_do_handle(message, ctx);
    }

    /**
     * Else it's a response.
     */
#if OEM_DEBUG
    printf("OEM >> ");
    const oem_binary_response* r = message;
    for (size_t i = 0; i < ctx->message_length - sizeof(r->responseId); i++)
        putc(r->response[i], stdout);
    printf("\n");
#endif

    ctx->task_level = TASK_RESPONSE;
    return ret;
}

typedef enum {
    STATE_SEARCH_AA = 0,
    STATE_SEARCH_44,
    STATE_SEARCH_12,
    STATE_FILL_PREP,
    STATE_FILL,
    STATE_FILL_VALIDATE,
    STATE_MESSAGE_READY,
} oem_task_state_t;

typedef enum {
    TARGET_NONE = 0, /* unused */
    TARGET_HEADER,
    TARGET_BODY,
    TARGET_CRC,
} oem_task_target_t;

/**
 * Reset the state machine to the initial state, i.e.,
 * first sync word search with no data in the buffer.
 */
static void sm_reset(oem_task_state_machine_t* sm)
{
    if (!sm)
        return;
    memset(sm, 0, sizeof(*sm));
    sm->state = STATE_SEARCH_AA;
}

/**
 * State machine core engine. Feeds the state machine until either:
 * 1) the input data is consumed all,
 *    where feed task is resumed when new data arrives and called again.
 * 2) a complete message is ready,
 *    where the message is to be passed to the handler layer.
 * 3) or an error is encountered.
 * 
 * Returns:
 * - Normal error codes (what's expected):
 *   OEM_OK: successful.
 *   OEM_ERR_LOG_TOO_LARGE: reply size exceeds interface's buffer capacity.
 *   OEM_ERR_LOG_HEADER_SIZE: reply-encoded header size is not sizeof(oem_binary_header_t).
 *   OEM_ERR_LOG_RESP_SIZE: response size is too short to be a valid response.
 *   OEM_ERR_LOG_CRC: CRC mismatch (validation state).
 *  
 * - Unlikely error codes (apparent bugs):
 *   OEM_ERR_NULL: any of the pointers is null.
 *   OEM_ERR_LOG_SM_STATE: invalid sm->state encountered.
 *   OEM_ERR_LOG_SM_TARGET: invalid sm->target encountered.
 *   OEM_ERR_LOG_SM_PREFILL: more bytes fed than expected (fill prep state).
 *   OEM_ERR_LOG_SM_SIZE_MISMATCH: fed bytes don't match the expected (validation state).
 */
static int sm_feed(oem_task_state_machine_t* sm,
                   const uint8_t* data,
                   size_t  len,
                   size_t* consumed,
                   oem_task_context_t* ctx)
{
    int ret = OEM_OK;
    size_t i = 0;
    
    if (!consumed)
        return OEM_ERR_NULL;
    
    if (!sm || !data || !ctx) {
        *consumed = 0;
        return OEM_ERR_NULL;
    }

    while (i < len || sm->state == STATE_FILL_VALIDATE) {
        switch (sm->state) {
        case STATE_SEARCH_AA:
            /* expect sync 1, proceed to sync 2 search on success */
            if (data[i++] == OEM_SYNC_BYTE1) {
                sm->buffer.header.sync[0] = OEM_SYNC_BYTE1;
                sm->state = STATE_SEARCH_44;
                sm->have = 1;
            }
            break;

        case STATE_SEARCH_44:
            /* expect sync 2, proceed to sync 3 search on success */
            if (data[i] == OEM_SYNC_BYTE2) {
                sm->buffer.header.sync[1] = OEM_SYNC_BYTE2;
                sm->state = STATE_SEARCH_12;
                sm->have = 2;
                i++;
            }
            else if (data[i] == OEM_SYNC_BYTE1)
                i++; /* still sync 1 candidate */
            else
                sm_reset(sm); /* no sync */
            break;

        case STATE_SEARCH_12:
            /* expect sync 3, proceed to header read on success */
            if (data[i] == OEM_SYNC_BYTE3) {
                sm->buffer.header.sync[2] = OEM_SYNC_BYTE3;
                sm->state = STATE_FILL_PREP;
                sm->target = TARGET_HEADER;
                sm->have = 3;
                i++;
            }
            /* else return to sync 1 or 2 search */   
            else if (data[i] == OEM_SYNC_BYTE1) {
                sm->state = STATE_SEARCH_44;
                sm->have = 1;
                i++;
            }
            else {
                sm_reset(sm);
            }
            break;

        case STATE_FILL_PREP:
        /**
         * Fill prep state:
         * - Set the expected/required sizes according to the target.
         * - Proceed to fill state.
         */
            {    
            size_t expected;
            switch (sm->target) {
            case TARGET_HEADER:
                expected = sizeof(oem_binary_header_t);
                break;
            case TARGET_BODY:
                expected = sizeof(oem_binary_header_t) 
                           + sm->buffer.header.messageLength;
                break;
            case TARGET_CRC:
                expected = sizeof(oem_binary_header_t) 
                           + sm->buffer.header.messageLength + sizeof(uint32_t);
                break;
            default:
                oem_debug_error("sm error: invalid target %d\n", sm->target);
                sm_reset(sm);
                ret = OEM_ERR_LOG_SM_TARGET;
                goto sm_feed_end;
            }

            if (sm->have > expected) {
                oem_debug_error("sm error: have %d exceeds expected sz %d\n",
                                sm->have, expected);
                sm_reset(sm);
                ret = OEM_ERR_LOG_SM_PREFILL;
                goto sm_feed_end;
            }

            if (expected > OEM_TASK_STATE_MACHINE_BUF_SIZE) {
                oem_debug_error("sm error: expected sz %d exceeds buf sz %d\n",
                                expected, OEM_TASK_STATE_MACHINE_BUF_SIZE);
                sm_reset(sm);
                ret = OEM_ERR_LOG_TOO_LARGE;
                goto sm_feed_end;
            }

            /* expect header, proceed to body read on success */
            sm->need = expected - sm->have;
            sm->state = STATE_FILL;
            }
            break;

        case STATE_FILL:
            /**
             * Fill state:
             * - Fill the buffer until the expected size is met.
             * - Proceed to validation if the expected size is met.
             */
            {
                size_t to_copy = len - i;
                if (to_copy > sm->need)
                    to_copy = sm->need;
                memcpy(sm->buffer.bytes + sm->have, data + i, to_copy);
                sm->have += to_copy;
                sm->need -= to_copy; 
                i += to_copy;
            }
            if (sm->need == 0)
                sm->state = STATE_FILL_VALIDATE;
            break;

        case STATE_FILL_VALIDATE:
        /**
         * Validate state:
         * - Validate the filled data according to the target and populate the context.
         *   1) header:
         *    - Validate the header size and message length.
         *    - Populate the context with the message ID, message length, and response bit.
         *   2) body:
         *    - Validate the fed bytes against the expected size.
         *    - If response, validate against the minimum response size.
         *    - Populate the context with the response ID if it's a response message.
         *    - Nothing is done for log messages.
         *   3) CRC:
         *    - Calculate the CRC, validate against the received one.
         *    - Validation skipped if the CRC was not read.
         *    - Populate the context with the calculated and received CRC values.
         */
            switch (sm->target) {
            case TARGET_HEADER:
                if (sm->buffer.header.headerLength != sizeof(oem_binary_header_t)) {
                    oem_debug_error("invalid header size: got %d (0x%x)\n",
                                    sm->buffer.header.headerLength,
                                    sm->buffer.header.headerLength);
                    sm_reset(sm);
                    ret = OEM_ERR_LOG_HEADER_SIZE;
                    goto sm_feed_end;
                }

                if (sm->have != sizeof(oem_binary_header_t)) {
                    oem_debug_error("sm error: have %d does not match header size %d\n",
                                    sm->have, sizeof(oem_binary_header_t));
                    sm_reset(sm);
                    ret = OEM_ERR_LOG_SM_SIZE_MISMATCH;
                    goto sm_feed_end;
                }
                
                oem_debug_info("New message header retrieved: rx status %08X\n",
                               sm->buffer.header.receiverStatus);

                ctx->message_id = sm->buffer.header.messageID;
                ctx->message_length = sm->buffer.header.messageLength;
                ctx->is_response = (sm->buffer.header.messageType & OEM_MSGTYPE_RESPONSE) 
                                    == OEM_MSGTYPE_RESPONSE;
                sm->target = TARGET_BODY;
                sm->state = STATE_FILL_PREP;
                break;

            case TARGET_BODY:
                if (sm->have != sizeof(oem_binary_header_t) + ctx->message_length) {
                    oem_debug_error("invalid log size: got %d, expected at least %d\n",
                                sm->have, sizeof(oem_binary_header_t) + ctx->message_length);
                    sm_reset(sm);
                    ret = OEM_ERR_LOG_SM_SIZE_MISMATCH;
                    goto sm_feed_end;
                }

                if (ctx->is_response) {
                    if (sm->have < sizeof(oem_binary_response)) {
                        oem_debug_error("invalid response size: got %d, expected at least %d\n",
                                    sm->have, sizeof(oem_binary_response));
                        sm_reset(sm);
                        ret = OEM_ERR_LOG_RESP_SIZE;
                        goto sm_feed_end;
                    }
                    oem_binary_response* r = (oem_binary_response*) sm->buffer.bytes;
                    ctx->response_id = r->responseId;
                }

                sm->target = TARGET_CRC;
                sm->state = STATE_FILL_PREP;
                break;

            case TARGET_CRC:
                if (ctx->crc_read_skipped == false) {
                    oem_crc crc_calced = oem_crc32(sm->buffer.bytes, sm->have - sizeof(oem_crc));
                    oem_crc crc_received;
                    memcpy(&crc_received, sm->buffer.bytes + sm->have - sizeof(oem_crc), sizeof(oem_crc));
                    ctx->crc_calced = crc_calced;
                    ctx->crc_received = crc_received;
                    if (crc_received != crc_calced) {
                        oem_debug_error("CRC verification failed: expected %08X, got %08X\n",
                                    crc_calced, crc_received);
                        sm_reset(sm);
                        ret = OEM_ERR_LOG_CRC;
                        goto sm_feed_end;
                    }
                }
                sm->state = STATE_MESSAGE_READY;
                goto sm_feed_end;
            
            default:
                oem_debug_error("sm error: invalid target %d\n", sm->target);
                sm_reset(sm);
                ret = OEM_ERR_LOG_SM_TARGET;
                goto sm_feed_end;
            }
        break;

        default:
            oem_debug_error("invalid state: %d\n", sm->state);
            sm_reset(sm);
            ret = OEM_ERR_LOG_SM_STATE;
            goto sm_feed_end;
        }
    }

sm_feed_end:
    *consumed = i;
    return ret;
}

static long ms_until(const struct timespec* deadline)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (deadline->tv_sec - now.tv_sec) * 1000
           + (deadline->tv_nsec - now.tv_nsec) / 1000000;
}

int oem_task_read_single_reply(int idx,
                               uint16_t timeout,
                               oem_task_context_t* ctx)
{
    struct timespec deadline;
    oem_task_state_machine_t* sm;
    size_t consumed = 0;
    oem_task_context_t ctx_local;
    bool using_local_ctx = false;
    int ret;

    /* ctx object is necessary for state propagation;
       use a local one if the user doesn't care for outputs */
    if (!ctx) {
        ctx = &ctx_local;
        using_local_ctx = true;
    }

    if (idx < 0 || idx >= OEM_IO_INTERFACES) {
        ret = OEM_ERR_IO_IFACE_INDEX;
        goto early_return_task;
    }

    memset(ctx, 0, sizeof(*ctx));

    clock_gettime(CLOCK_MONOTONIC, &deadline);
    deadline.tv_sec += timeout / 1000;
    deadline.tv_nsec += (timeout % 1000) * 1000000;
    if (deadline.tv_nsec >= 1000000000) {
        deadline.tv_sec += 1;
        deadline.tv_nsec -= 1000000000;
    }

    sm = &state_machines[idx];
    sm_reset(sm);

    /* If we have leftover bytes in the interface buffer, feed them first. */
    size_t available = oem_io_available(idx);     
    if (available) {
        const void* d; size_t s;
        if ((ret = oem_io_peek(idx, &d, &s)) != OEM_OK) {
            oem_debug_error("Failed to peek I/O buffer for port %d: %d\n", idx, ret);
            goto early_return_task;
        }
        ret = sm_feed(sm, d, s, &consumed, ctx);
        if (ret != OEM_OK && consumed == 0)
            consumed = 1; /* Make sure we make progress */
        oem_io_consume(idx, consumed);

        if (ret != OEM_OK) {
            oem_debug_error("State machine feed error for port %d: %d\n", idx, ret);
            goto early_return_task;
        }
    }
    
    if (sm->state != STATE_MESSAGE_READY) {
        /**
         * A complete message has not arrived yet.
         * Read more from the I/O layer.
         */
        while (1) {
            long remaining_ms = ms_until(&deadline);
            if (remaining_ms <= 0)
                break;

            ret = oem_io_fill(idx, remaining_ms);
            if (ret == OEM_ERR_IO_TIMEOUT)
                continue; /* no bytes yet; the deadline decides when to stop */
            if (ret != OEM_OK && ret != OEM_ERR_FULL) {
                sm_reset(sm);
                goto early_return_task;
            }

            /* feed the state machine with the newly arrived data */
            available = oem_io_available(idx);
            if (available == 0)
                continue; /* no data available, try again */

            const void* d; size_t s;
            if ((ret = oem_io_peek(idx, &d, &s)) != OEM_OK) {
                oem_debug_error("Failed to peek I/O buffer for port %d: %d\n", idx, ret);
                goto early_return_task;
            }

            ret = sm_feed(sm, d, s, &consumed, ctx);
            if (ret != OEM_OK && consumed == 0)
                consumed = 1; /* Make sure we make progress */
            oem_io_consume(idx, consumed);

            if (ret != OEM_OK)
                goto early_return_task;

            if (sm->state == STATE_MESSAGE_READY)
                break; /* message ready */
        }
    }

    if (sm->state == STATE_FILL && sm->target == TARGET_CRC) {
        /**
         * CRC was expected but not read within timeout.
         * 
         * OEM7s occasionally fail to report the CRC trailer. Instead of
         * discarding the message, we mark this on the context to allow
         * certain handlers to process it at the risk of bypassing the
         * validation. This behavior can be enabled by calling
         * oem_log_ignore_missing_crc().
         */
        ctx->crc_read_skipped = true;
        sm->state = STATE_MESSAGE_READY;

        oem_debug_warning("CRC read skipped for port %d: MID %d\n",
                          idx, ctx->message_id);

        if (using_local_ctx == false) {
            /* Populate the context with the CRC info for reference, if care. */
            ctx->crc_calced = oem_crc32(sm->buffer.bytes,
                                        sizeof(oem_binary_header_t) + ctx->message_length);
            ctx->crc_received = 0; /* sentinel */
        }
    }

    if (sm->state == STATE_MESSAGE_READY) {
        /* A new message is ready to be processed. */
        ret = process_reply(sm->buffer.bytes, ctx);
        sm_reset(sm);
        return ret;
    }

    /* If neither a new frame nor CRC read fail, it's timeout. */
    sm_reset(sm);
    ret = OEM_ERR_IO_TIMEOUT;

early_return_task:
    ctx->task_level = TASK_MAIN;
    return ret;
}
