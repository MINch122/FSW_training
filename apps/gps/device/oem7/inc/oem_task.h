/**
 * @file oem_task.h
 * @brief OEM7 log receiving tasks.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2024.
 */
#ifndef _OEM_TASK_H_
#define _OEM_TASK_H_

#include "oem_config.h"
#include "oem_types.h"

typedef enum {
    TASK_MAIN       = 1, /* Returned from oem_task_read_single_reply. */
    TASK_RESPONSE   = 2, /* Returned from process_reply (response handler). */
    TASK_LOG        = 3, /* Returned from oem_log_do_handle (log handler). */
    TASK_CALLBACK   = 4, /* Returned from a log handler callback. */
} oem_task_tasklevel;
 
typedef struct {
    /* handling context */
    uint8_t     task_level;     /* Last called handling procedure. See oem_task_tasklevel. */
    int         cb_exec_count;  /* Number of executed log callbacks.    */

    /* message info */
    oem_short   message_id;     /* Message ID.                          */
    oem_enum    response_id;    /* Response ID, if it was a Response.   */
    oem_ushort  message_length; /* Body length in bytes.                */
    oem_crc     crc_received;   /* CRC at the tail of the message.      */
    oem_crc     crc_calced;     /* CRC calculated from the message.     */

    /* flags */
    bool        is_response;    /* Was it a Response message?           */
    bool        crit_failure;   /* Did handler go broken?               */
    bool        crc_read_skipped;    /* Was the CRC read skipped?       */
} oem_task_context_t;

/**
 * @brief Process a single reply message from the OEM receiver.
 * 
 * @details
 *       - Drives the full reply pipeline for a single message: pulls bytes
 *         from the I/O interface buffer, assembles them into a complete 
 *         packet via the state machine, then dispatches the packet either as
 *         a response or through the matching log handler and its registered
 *         callbacks.
 *
 *       - Because the call climbs several layers up, the meaning of a non-OK
 *         return depends on where it gave up. Inspect @a ctx after the call:
 *         @c ctx->task_level identifies the layer (TASK_MAIN / TASK_RESPONSE /
 *         TASK_LOG / TASK_CALLBACK), and the remaining ctx fields carry
 *         whatever the layer learned before returning.
 * 
 *       - The state machine does not hold any state between calls, i.e., each
 *         call resets the previous parsing process.
 *
 * @param iface_idx  I/O interface index to read from. Use the designated
 *                   indices from oem_io_init_interface().
 * @param timeout    Timeout in milliseconds to wait for a complete message.
 * @param[out] ctx   Log handling context. After the call, this will contain
 *                   the message info and handling context. Null allowed.
 *
 * @return OEM_OK: Successful.
 *
 *       - If returned from the task layer (ctx->task_level == TASK_MAIN):
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 *         OEM_ERR_IO_IFACE_UNSET: The interface is not initialized.
 *         OEM_ERR_IO_TIMEOUT: No complete message was read before the timeout.
 *         OEM_ERR_LOG_HEADER_SIZE: encoded header size field does not match
 *                                  the expected size.
 *         OEM_ERR_LOG_RESP_SIZE: Response message shorter than the minimum.
 *         OEM_ERR_LOG_TOO_LARGE: message length exceeds the SM buffer size.
 *         OEM_ERR_LOG_CRC: CRC mismatch.
 *         Or any other negative code returned by the underlying read callback.
 * 
 *       - If returned from the response handler (ctx->task_level == TASK_RESPONSE):
 *         Currently no error is returned from this layer.
 * 
 *       - If returned from the handler layer (ctx->task_level == TASK_LOG):
 *         OEM_ERR_LOG_STRAY: No handler registered for this log message.
 *         OEM_ERR_LOG_BODY_SIZE: Message size doesn't match the handler's expectation.
 *         OEM_ERR_LOG_MISSING_CRC: CRC was expected but did not arrive within timeout.
 *         OEM_ERR_NOBUF: Handler's recent message buffer is not set (critical).
 *         
 *       - If returned from a log handler callback (ctx->task_level == TASK_CALLBACK):
 *         OEM_ERR_NOT_FOUND: Callback node exists but the callback is null (critical).
 *         OEM_ERR_UTILS_LIST_NULL: Callback list is null (critical).
 *         Or any other error code returned by the callback itself.
 * 
 *       - Unexpected error codes during normal operation:
 *         OEM_ERR_NULL: required pointer argument is null.
 *         OEM_ERR_LOG_SM_STATE: invalid sm->state encountered.
 *         OEM_ERR_LOG_SM_TARGET: invalid sm->target encountered.
 *         OEM_ERR_LOG_SM_PREFILL: more bytes fed than expected (fill prep state).
 *         OEM_ERR_LOG_SM_SIZE_MISMATCH: fed bytes don't match the expected (validation state).
 */
int oem_task_read_single_reply(int iface_idx,
                               uint16_t timeout,
                               oem_task_context_t* ctx);

#endif
