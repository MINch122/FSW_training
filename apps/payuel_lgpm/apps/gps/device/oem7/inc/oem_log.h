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
 * @brief Log handling callback function type.
 * 
 * @details A log callback takes a full LOG message (including the header) and
 *          must return a oem_ret error code or a user-defined return value.
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
    uint32_t    logCount;
    uint32_t    logErrCount;
    uint8_t     errorCause; /* Last reported error code (oem_ret). */
} oem_log_handler_stat_t;

/**
 * @brief Log handler housekeeping.
 */
typedef struct {
    oem_ushort  messageId;
    oem_ushort  messageLength;
    uint32_t    recentMsgTimeStamp;
    int         attachedCallbacks;
    uint32_t    logCount;
    uint32_t    logErrCount;
    uint8_t     errorCause;
    uint8_t     status;
    bool        ignoreChecksum;
    char        name[OEM_LOG_HANDLER_NAME_LEN];
} oem_log_handler_hk_t;


/**
 * Pass this to OEM_Log_RegisterHandler() if the expected log length is
 * variable, e.g., it has per-satellite components.
 */
#define OEM_LOG_HANDLER_MLEN_VARIABLE   0


/**
 * @brief Log handler status. See OEM_Log_SetHandlerStatus() and similar methods.
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
 * @brief   Initialize the handler mutex.
 * 
 * @details Handler mutex prevents race conditions when handler objects or
 *          their attributes are retrieved or altered. Log handling itself
 *          does not lock the mutex.
 * 
 *          Lock/unlock return values are not checked internally; i.e., handler 
 *          methods will still operate even if the mutex initialization failes 
 *          or gets skipped. It is still recommended to properly call this 
 *          method at the Application initialization step.
 *   
 * @return  OEM_OK: Successful.
 *          OEM_ERR_MUTEX_INIT: pthread mutex init failed.
 */
int OEM_Log_HandlerInit(void);

/**
 * @brief Register a new empty handler for a Log Message. 
 *        The new handler has an INACTIVE status and no callbacks, meaning only
 *        statistics are updated as messages arrive. For actual processing the
 *        user must attach callbacks and then manually activate the handler by
 *        ground commands.
 * 
 * @details
 *        1) The name string has a length limit of OEM_LOG_HANDLER_NAME_LEN.
 *        2) Incoming messages will be truncated by a size limit of 
 *           OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE.
 *        3) Maximum number of handlers is defined by OEM_LOG_HANDLER_MAX.
 * 
 * @param name Handler name.
 * @param mid Message ID.
 * @param mlen Message length. Header inclusive, CRC exclusive. Set this 0 if
 *             the message has variable-lengths. 
 * @return OEM_OK: Successful.
 *         OEM_ERR_EXISTS: This @a mid has already been registered.
 *         OEM_ERR_FULL: Handler slot is full.
 *         OEM_ERR_NOMEM: Could not allocate the message buffer (malloc).
 */
int OEM_Log_RegisterHandler(const char* name,
                            oem_ushort mid,
                            oem_ushort mlen);

/**
 * @brief Unregister an existing handler to free a handler slot.
 * 
 * @param mid Message ID to unregister.
 * @return OEM_OK: Successful. 
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 */
int OEM_Log_UnregisterHandler(oem_ushort mid);

/**
 * @brief Add a message processing callback to a handler. Callbacks are
 *        sequentially executed as a message arrives, in the order of
 *        addtion. The handler must be active for the callbacks to run.
 * 
 * @details
 *        Callbacks are added as a linked-list node. If a callback returns
 *        any other values than OEM_OK, the rest of the callbacks won't be
 *        called.
 *        Handler mutex is not locked at the moment of callback execution.
 * 
 * @param mid Message ID of the handler to add the callback to.
 * @param callback A callback function. See oem_log_callback_t.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_LIST: Callback list-put failed.
 */
int OEM_Log_AddCallback(oem_ushort mid,
                        oem_log_callback_t callback);

int OEM_Log_ClearCallbacks(oem_ushort mid);

/**
 * @brief Set a handler running status. See oem_log_handler_status_t for details.
 * 
 * @details
 *        1) An active/inactive handler can only transition to active/inactive
 *           or dormant.
 *        2) A dormant handler can only be "awaken" to inactive by calling
 *           OEM_Log_HandlerWakeup().
 *        3) No status can go broken unless by OEM_Log_HandlerSetBroken().
 *        4) Setting @a override to true ignores all the limits above. This is
 *           the only way for a handler to escape from HANDLER_BROKEN, and must
 *           be carefully applied by ground operators.
 * 
 * @param mid Message ID of the handler.
 * @param status Destination status.
 * @param override: Force the status transition (not recommended).
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_RANGE: No such @a status is defined.
 *         OEM_ERR_INVALID: Invalid transition (only when @a override is false).
 */
int OEM_Log_SetHandlerStatus(oem_ushort mid,
                             uint8_t status,
                             bool override);

/**
 * @brief Get the running status from a handler.
 * 
 * @param mid Message ID of the handler.
 * @param[out] status Current handler status.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a status is null.
 */
int OEM_Log_GetHandlerStatus(oem_ushort mid,
                             uint8_t* status);

/**
 * @brief Activate a handler from the inactive status.
 * 
 * @param mid Message ID of the handler.
 * @return See OEM_Log_SetHandlerStatus().
 */
int OEM_Log_HandlerActivate(oem_ushort mid);

/**
 * @brief Dectivate a handler from the active status.
 * 
 * @param mid Message ID of the handler.
 * @return See OEM_Log_SetHandlerStatus().
 */
int OEM_Log_HandlerDeacivate(oem_ushort mid);

/**
 * @brief Make a handler dormant from the active/inactive status.
 * 
 * @param mid Message ID of the handler.
 * @return See OEM_Log_SetHandlerStatus().
 */
int OEM_Log_HandlerGoDormant(oem_ushort mid);

/**
 * @brief Wake up a handler from its dormant status and turn it inactive.
 * 
 * @param mid Message ID of the handler.
 * @return See OEM_Log_SetHandlerStatus().
 */
int OEM_Log_HandlerWakeup(oem_ushort mid);

/**
 * @brief Activate all deactive handlers.
 * 
 * @return OEM_OK (never fails).
 */
int OEM_Log_HandlerActivateAll(void);

/**
 * @brief Dectivate all active handlers.
 * 
 * @return OEM_OK (never fails).
 */
int OEM_Log_HandlerDeactivateAll(void);

/**
 * @brief Mark a handler as broken.
 * 
 * @param mid Message ID of the handler.
 * @return See OEM_Log_SetHandlerStatus().
 */
int OEM_Log_HandlerSetBroken(oem_ushort mid);

/**
 * @brief Retrieve the registered message length from a handler. The length
 *        should be header inclusive, CRC exclusive.
 * 
 * @param mid Message ID of the handler.
 * @param[out] mlen Registered message length. 0 if variable-lengthed.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a mlen is null.
 */
int OEM_Log_GetMessageLength(oem_ushort mid, oem_ushort* mlen);

/**
 * @brief Retrieve message statistics.
 * 
 * @param mid Message ID of the handler.
 * @param[out] stat Message counters.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a stat is null.
 */
int OEM_Log_GetMessageStatistics(oem_ushort mid, oem_log_handler_stat_t* stat);

/**
 * @brief Retrieve handler name.
 * 
 * @param mid Message ID of the handler.
 * @param[out] name Handler name buffer, at least a size of
 *                  OEM_LOG_HANDLER_NAME_LEN.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a name is null.
 */
int OEM_Log_GetHandlerName(oem_ushort mid, char* name);

/**
 * @brief Reset message statistics.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 */
int OEM_Log_ResetHandlerCounters(oem_ushort mid);

/**
 * @brief Retrieve a handler housekeeping bundle.
 * 
 * @param mid Message ID of the handler.
 * @param[out] hk Handler housekeeping.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_NULL: @a hk is null.
 */
int OEM_Log_GethandlerHousekeeping(oem_ushort mid,
                                   oem_log_handler_hk_t* hk);

/**
 * @brief Copy the latest received message from a handler to @a buffer.
 *        If @a bufsize is smaller than the actual message length, the copy
 *        will be truncated accordingly.
 * 
 * @param mid Message ID of the handler.
 * @param[out] buffer Buffer to copy the recent message. @nonnull.
 * @param bufsize Size of @a buffer.
 * @param[out] copiedSize Actually copied bytes. NULL allowed.     
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 *         OEM_ERR_EMPTY: There is no recent message (the message buffer does
 *                        does not hold a valid sync bytes).
 *         OEM_ERR_NULL: @a buffer is null.
 */
int OEM_Log_DumpRecentMessage(oem_ushort mid,
                              void* buffer,
                              size_t bufsize,
                              size_t* copiedSize);

/**
 * @brief Enable the CRC verification.
 *        After the call, the handler calculates the message CRC and reject
 *        processing if the trailing CRC is absent or does not match the
 *        expected value.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 */
int OEM_Log_EnableCsVerification(oem_ushort mid);

/**
 * @brief Disable the CRC verification.
 *        After the call, the handler simply ignores the trailing CRC
 *        regardless of whether it was read.
 * 
 * @param mid Message ID of the handler.
 * @return OEM_OK: Successful.
 *         OEM_ERR_NOTFOUND: There is no handler for @a mid.
 */
int OEM_Log_DisableCsVerification(oem_ushort mid);

/**
 * @brief Lock the handler access mutex. The mutex is locked/unlocked
 *        internally during the handling processes. It is not recommended to
 *        call this in a user context.
 * 
 * @return Depends on the implementation. See OEM_MutexLock().
 */
// int OEM_Log_LockHandlers(void);

/**
 * @brief Unlock the handler access mutex. The mutex is locked/unlocked
 *        internally during the handling processes. It is not recommended to
 *        call this in a user context.
 * 
 * @return Depends on the implementation. See OEM_MutexLock().
 */
// int OEM_Log_UnlockHandlers(void);

#endif