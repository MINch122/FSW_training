/**
 * @file oem_log.h
 * @brief OEM7 log handler.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_LOG_H_
#define _OEM_LOG_H_

#include "oem_config.h"
#include "msg/oem_msg_common.h"


/**
 * Pass this to oem_log_handler_register() if the expected log length is
 * variable, e.g., it has per-satellite components.
 */
#define OEM_LOG_HANDLER_MLEN_VARIABLE   0


/**
 * @brief Log handling callback function type.
 * 
 * @details A log callback takes a full LOG message (including the header) and
 *          must return a oem_ret_t error code or a user-defined return value.
 * 
 *          If a callback returns any other values than OEM_OK, the handling
 *          process will return immediately and any remaining callbacks
 *          attached to the handler will be ignored.
 * 
 *          At the moment of execution, @a msg is always the recent message
 *          buffer object in the parent handler. It is not recommended to
 *          directly modify @a msg unless necessary.
 */
typedef int (*oem_log_callback_t)(void* msg);

/**
 * @brief Log handler object (opaque).
 */
typedef struct oem_log_handler_s oem_log_handler_t;

/**
 * @brief Log handler message statistics.
 */
typedef struct {
    uint32_t    log_count;
    uint32_t    log_err_count;
    uint32_t    recent_msg_timestamp;
    int         error_cause; /* Last reported error code (oem_ret_t). */
} oem_log_stat_t;

/**
 * @brief Log handler housekeeping.
 */
typedef struct __attribute__((packed)) {
    oem_ushort  message_id;
    oem_ushort  message_length;
    uint32_t    recent_msg_timestamp;
    int         callbacks;
    uint32_t    log_count;
    uint32_t    log_err_count;
    int         error_cause; /* Last reported error code (oem_ret_t). */
    uint8_t     status;
    bool        ignore_checksum;
    char        name[OEM_LOG_HANDLER_NAME_LEN];
} oem_log_handler_hk_t;


/**
 * @brief Log handler status. See oem_log_handler_set_status() and similar methods.
 */
typedef enum __attribute__((packed)) {
    /**
     * Handler is empty and needs to be registered.
     */
    HANDLER_EMPTY       = 0,

    /**
     * Handler is active. Incoming messages are processed by
     * the callbacks attached to this handler.
     */
    HANDLER_ACTIVE      = 1,

    /**
     * Handler is inactive. Callbacks won't be called but 
     * log statistics are updated by incoming messages.
     */
    HANDLER_INACTIVE    = 2,

    /**
     * Dormant status completely ignores messages.
     */
    HANDLER_DORMANT     = 3,
    
    /**
     * This handler has broken settings and needs to be intervened
     * by a ground operator. This flag should be set by a driver
     * command or a handler callback in case of any anomalies.
     */
    HANDLER_BROKEN      = 4,

} oem_log_handler_status_t;


/**
 * @brief Initialize the log layer.
 * 
 * @details
 *      - This function must be called before any other log handler functions.
 *      - This function initializes the internal mutex for handler operations.
 *      - Successful mutex lock is not checked internally. The log layer will
 *            still run regardless of the mutex status.
 *   
 * @return  OEM_OK: Successful.
 *          OEM_ERR_MUTEX_INIT: pthread mutex init failed.
 */
int oem_log_init(void);


/* ════════════════════════════════════════════════════════════════════════
 *  Log handler registration
 * ════════════════════════════════════════════════════════════════════════ */


/**
 * @brief Register a new empty handler for a Log Message. 
 *
 *        The new handler has an INACTIVE status and no callbacks, meaning only
 *        counters are updated as messages arrive. For actual processing the
 *        user must attach callbacks and manually activate the handler by
 *        ground commands.
 * 
 * @details
 *        - Maximum number of handlers is defined by OEM_LOG_HANDLER_MAX.
 *        - Only one handler can be registered per a Message ID.
 *        - @a mlen is checked against the actual message length in the
 *              header. The handler will reject messages with incorrect
 *              lengths. For variable-length messages, set @a mlen to 
 *              OEM_LOG_HANDLER_MLEN_VARIABLE, whose maximum length is
 *              defined by OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE. If the message
 *              length is greater than this maximum, it will be truncated.
 * 
 * @param name Handler name, shorter than OEM_LOG_HANDLER_NAME_LEN.
 * @param mid  Log Message ID for this handler.
 * @param mlen Message length. Header inclusive, CRC exclusive. Set this to
 *             OEM_LOG_HANDLER_MLEN_VARIABLE if the length is variable.
 * @return OEM_OK: Successful.
 *         OEM_ERR_EXISTS: This @a mid has already been registered.
 *         OEM_ERR_FULL: Handler slot is full.
 *         OEM_ERR_NOMEM: Could not allocate the message buffer (malloc).
 */
int oem_log_handler_register(const char* name,
                             oem_ushort mid,
                             oem_ushort mlen);

/**
 * @brief Unregister an existing handler to free the handler slot.
 * 
 * @param mid Message ID to unregister.
 * @return OEM_OK: Successful. 
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 */
int oem_log_handler_unregister(oem_ushort mid);


/* ════════════════════════════════════════════════════════════════════════
 *  Log handler status management.
 *  
 *  - A handler can have one of the 4 statuses: ACTIVE, INACTIVE, DORMANT,
 *      and BROKEN.
 *  1) ACTIVE is a normal status. The handler will, upon receiving a message:
 *      - Validate the message
 *      - Update the log statistics (e.g., log count, error count, etc.)
 *      - Execute the attached callbacks.
 *  2) INACTIVE is a passive status. The handler will, upon receiving a message:
 *      - Validate the message
 *      - Update the log statistics and return.
 *  3) DORMANT handlers completely ignore incoming messages (nothing happens).
 *  4) BROKEN handlers also ignore incoming messages, but they are intended to
 *      indicate an anomaly during processing, typically set by a callback.
 * 
 *  - Status transition rules:
 *  1) ACTIVE status can enter INACTIVE or DORMANT status.
 *  2) INACTIVE status can enter ACTIVE or DORMANT status.
 *  3) DORMANT status can only enter to INACTIVE status, by calling
 *       oem_log_handler_wakeup().
 *  4) BROKEN status can only be set by calling oem_log_handler_mark_broken(),
 *       and can only be escaped by calling oem_log_handler_set_status() with
 *       @a override set to true.
 * 
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Activate a handler from the INACTIVE status.
 * 
 * @details
 *      - Handlers with ACTIVE status will execute the attached callbacks as
 *          messages arrive.
 *      - Only handlers with INACTIVE status can be activated. To change the
 *          status from DORMANT to ACTIVE, call oem_log_handler_wakeup() first.
 * 
 * @param mid Message ID of the handler.
 * @return See oem_log_set_handler_status().
 */
int oem_log_handler_activate(oem_ushort mid);

/**
 * @brief Deactivate a handler from the ACTIVE status.
 * 
 * @details
 *     - Handlers with INACTIVE status will not execute the attached callbacks,
 *         but the log stats will still be updated as messages arrive.
 * 
 * @param mid Message ID of the handler.
 * @return See oem_log_set_handler_status().
 */
int oem_log_handler_deactivate(oem_ushort mid);

/**
 * @brief Make a handler dormant from the ACTIVE/INACTIVE status.
 * 
 * @details
 *    - Handlers with DORMANT status will completely ignore incoming messages,
 *        as if they were not received, i.e., the log stats are not updated.
 *    - DORMANT status is designed for a handler to be temporarily disabled
 *        without losing the handler settings (e.g., attached callbacks). To 
 *        wake up a dormant handler, call oem_log_handler_wakeup().
 * 
 * @param mid Message ID of the handler.
 * @return See oem_log_set_handler_status().
 */
int oem_log_handler_go_dormant(oem_ushort mid);

/**
 * @brief Wake up a handler from its DORMANT status and turn it INACTIVE.
 * 
 * @param mid Message ID of the handler.
 * @return See oem_log_set_handler_status().
 */
int oem_log_handler_wakeup(oem_ushort mid);

/**
 * @brief Activate all INACTIVE handlers.
 * 
 * @return OEM_OK (never fails).
 */
int oem_log_handler_activate_all(void);

/**
 * @brief Deactivate all ACTIVE handlers.
 * 
 * @return OEM_OK (never fails).
 */
int oem_log_handler_deactivate_all(void);

/**
 * @brief Mark a handler as broken.
 * 
 * @details
 *     - Handlers with BROKEN status completely ignore incoming messages, and
 *         the log stats are not updated. The behavior is identical to DORMANT.
 *     - BROKEN status is designed to be set by a driver command or a handler
 *         to inform ground operators of an anomaly in log handling. Call this
 *         in the callback context.
 *     - The only way to escape this is by calling oem_log_handler_set_status()
 *         with @a override set to true.
 * 
 * @param mid Message ID of the handler.
 * @return See oem_log_set_handler_status().
 */
int oem_log_handler_mark_broken(oem_ushort mid);

/**
 * @brief Set a handler running status. See oem_log_handler_status_t for details.
 * 
 * @details
 *     - Using this function is not recommended for normal status transitions.
 *         Instead, use the specific transition functions like
 *         oem_log_handler_activate(), oem_log_handler_deactivate().
 *     - Setting @a override to true allows any status transition, intended as
 *         a plumbing command for ground operators.
 * 
 * @param mid Message ID of the handler.
 * @param status Destination status.
 * @param override: Force the status transition (not recommended).
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_RANGE: No such @a status is defined.
 *         OEM_ERR_INVALID: Invalid transition (only when @a override is false).
 */
int oem_log_handler_set_status(oem_ushort mid,
                               uint8_t status,
                               bool override);

/**
 * @brief Get the running status from a handler.
 * 
 * @param mid Message ID of the handler.
 * @param[out] status Current handler status.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a status is null.
 */
int oem_log_handler_get_status(oem_ushort mid,
                               uint8_t* status);


/* ════════════════════════════════════════════════════════════════════════
 *  Log Message processing
 * ════════════════════════════════════════════════════════════════════════ */


/**
 * @brief Add a message processing callback to a handler. Callbacks are
 *        sequentially executed as a message arrives, in the order of
 *        addtion. The handler must be ACTIVE for the callbacks to run.
 * 
 * @details
 *        - Callbacks are added as a linked-list node.
 *        - If a callback returns any other values than OEM_OK, the rest of
 *            the callbacks won't be called.
 *        - Handler mutex is not locked at the moment of callback execution.
 * 
 * @param mid Message ID of the handler.
 * @param callback A callback function. See oem_log_callback_t.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_LOG_LIST: Callback list-put failed.
 */
int oem_log_add_callback(oem_ushort mid,
                         oem_log_callback_t callback);

/**
 * @brief Clear all callbacks attached to a handler.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 */
int oem_log_clear_callbacks(oem_ushort mid);

/**
 * @brief Retrieve message statistics.
 * 
 * @param mid Message ID of the handler.
 * @param[out] stat Message counters.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a stat is null.
 */
int oem_log_get_stat(oem_ushort mid, oem_log_stat_t* stat);

/**
 * @brief Reset message statistics to zeros.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 */
int oem_log_reset_stat(oem_ushort mid);

/**
 * @brief Retrieve the handler housekeeping bundle.
 * 
 * @param mid Message ID of the handler.
 * @param[out] hk Handler housekeeping.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a hk is null.
 */
int oem_log_get_handler_hk(oem_ushort mid,
                           oem_log_handler_hk_t* hk);

/**
 * @brief Copy the latest received message from a handler to @a buffer.
 *        If @a limit is smaller than the actual message length, the copy
 *        will be truncated accordingly.
 * 
 * @param mid         Message ID of the handler.
 * @param[out] buffer Buffer to copy the recent message.
 * @param offset      Offset in the message to start copying.
 * @param limit       Available size @a buffer.
 * @param[out] copied Actually copied bytes. NULL allowed.     
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_EMPTY: There is no recent message.
 *         OEM_ERR_NULL: @a buffer is null.
 */
int oem_log_get_recent_message(oem_ushort mid,
                                void*   buffer,
                                size_t  offset,
                                size_t  limit,
                                size_t* copied);

/**
 * @brief Get the registered message length for a handler.
 * 
 * @param mid Message ID of the handler.
 * @param[out] mlen Registered message length. 0 if variable-lengthed.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a mlen is null.
 */
int oem_log_get_message_length(oem_ushort mid, oem_ushort* mlen);

/**
 * @brief Retrieve handler name.
 * 
 * @param mid Message ID of the handler.
 * @param[out] name Handler name buffer, at least a size of
 *                  OEM_LOG_HANDLER_NAME_LEN.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a name is null.
 */
int oem_log_get_handler_name(oem_ushort mid, char* name);

/**
 * @brief Ignore future CRC read failures for a log.
 * 
 * @details
 *    - Some logs may have missing CRC trailers due to poor interface conditions.
 *        This function allows the handler to ignore CRC read failures and process
 *        the messages as usual.
 *    - CRC mismatch will still be rejected as a CRC error. This function only
 *        affects CRC read failures.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 */
int oem_log_ignore_missing_crc(oem_ushort mid);

/**
 * @brief Respect CRC read failures, and reject them as errors.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOT_FOUND: There is no handler for @a mid.
 */
int oem_log_reject_missing_crc(oem_ushort mid);


/**
 * @brief Lock the handler access mutex. The mutex is locked/unlocked
 *        internally during the handling processes. It is not recommended to
 *        call this in a user context.
 * 
 * @return Depends on the implementation. See OEM_MutexLock().
 */
// int oem_log_lock_handlers(void);

/**
 * @brief Unlock the handler access mutex. The mutex is locked/unlocked
 *        internally during the handling processes. It is not recommended to
 *        call this in a user context.
 * 
 * @return Depends on the implementation. See OEM_MutexLock().
 */
// int oem_log_unlock_handlers(void);


#endif
