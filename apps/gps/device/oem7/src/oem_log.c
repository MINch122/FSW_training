/**
 * @file oem_log.c
 * @brief OEM7 log handler.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include "oem_log.h"
#include "oem_task.h"  /* oem_task_context_t */
#include "oem_utils.h" /* mutex */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#if OEM_DEBUG
#include <stdio.h>
#include <errno.h>
#include <string.h>
#endif

#define WEEK2SEC 604800U

struct oem_log_handler_s {
    char                  name[OEM_LOG_HANDLER_NAME_LEN];
    oem_list_t*           callbacks;
    void*                 recent_message;
    oem_log_stat_t        stat;
    oem_ushort            message_id;
    oem_ushort            message_length;
    uint8_t               status;
    bool                  has_recent_message;
    bool                  ignore_missing_crc;
};

static oem_log_handler_t log_handlers[OEM_LOG_HANDLER_MAX];

static pthread_mutex_t hmut;

int oem_log_init(void)
{
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr) != 0)
        return OEM_ERR_LOG_MUTEX_INIT;

    if (pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT)    != 0 ||
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0 ||
            pthread_mutex_init(&hmut, &attr) != 0) {
        pthread_mutexattr_destroy(&attr);
#if OEM_DEBUG
        oem_debug_error("Failed to initialize log handler mutex: %s (%d)\n",
                         strerror(errno), errno);
#endif
        return OEM_ERR_LOG_MUTEX_INIT;
    }

    return OEM_OK;
}

int oem_log_handler_lock(void)
{
    return pthread_mutex_lock(&hmut) == 0 ?
           OEM_OK :
           OEM_ERR_LOG_MUTEX_LOCK;
}

int oem_log_handler_unlock(void)
{
    return pthread_mutex_unlock(&hmut) == 0 ?
           OEM_OK :
           OEM_ERR_LOG_MUTEX_UNLOCK;
}

static oem_log_handler_t* get_empty_handler_slot(void)
{
    for (int i = 0; i < OEM_LOG_HANDLER_MAX; ++i)
        if (log_handlers[i].status == HANDLER_EMPTY)
            return &log_handlers[i];
    return NULL;
}

static oem_log_handler_t* get_handler_by_id(oem_ushort id)
{
    if (id == 0)
        return NULL;
    for (int i = 0; i < OEM_LOG_HANDLER_MAX; ++i)
        if (log_handlers[i].status != HANDLER_EMPTY &&
            log_handlers[i].message_id == id)
                return &log_handlers[i];
    return NULL;
}

static void purge_handler(oem_log_handler_t* handler)
{
    if (!handler)
        return;
    if (handler->recent_message) {
        free(handler->recent_message);
    }
    if (handler->callbacks) {
        oem_list_free(handler->callbacks);
    }
    memset(handler, 0, sizeof(*handler));
}

int oem_log_handler_register(const char* name,
                            oem_ushort id,
                            oem_ushort mlen)
{
    oem_log_handler_t* newHandler = NULL;
    int ret = OEM_OK;

    oem_log_handler_lock();

    if (get_handler_by_id(id)) {
        oem_debug_error("Handler for MID %d has already been registered.\n",
                        id);
        ret = OEM_ERR_EXISTS;
    }
    else if ((newHandler = get_empty_handler_slot()) == NULL) {
        oem_debug_error("Handler queue is full: mid %d.\n",
                        id);
        ret = OEM_ERR_FULL;
    }
    else if ((newHandler->callbacks = oem_list_create()) == NULL) {
        ret = OEM_ERR_NOMEM;
    }
    else if (
        (newHandler->recent_message = malloc(mlen > 0 ?
                                             mlen :
                                             OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE)
        ) == NULL
    ) {
        oem_debug_error("malloc failed for handler ID %d.\n",
                        id);
        oem_list_free(newHandler->callbacks);
        ret = OEM_ERR_NOMEM;
    }
    else {
        strncpy(newHandler->name, name ? name : "", sizeof(newHandler->name));
        newHandler->name[sizeof(newHandler->name) - 1] = '\0';
        memset(&newHandler->stat, 0, sizeof(newHandler->stat));
        newHandler->message_id = id;
        newHandler->message_length = mlen;
        newHandler->status = HANDLER_INACTIVE;
        newHandler->has_recent_message = false;
    }

    oem_log_handler_unlock();
    return ret;

}

int oem_log_handler_unregister(oem_ushort id)
{
    oem_log_handler_t* handler;
    int ret = OEM_OK;

    oem_log_handler_lock();

    if ((handler = get_handler_by_id(id)) == NULL) {
        oem_debug_error("No handler for MID %d found.\n",
                        id);
        ret = OEM_ERR_NOT_FOUND;
    }
    else {
        purge_handler(handler);
    }

    oem_log_handler_unlock();
    return ret;
}

static bool is_valid_transfer(uint8_t from, uint8_t to, bool wakeup)
{
    /**
     * No status can be autonomously transfered to or from EMPTY/BROKEN.
     */
    if (from == HANDLER_EMPTY  ||
        from == HANDLER_BROKEN ||
        to == HANDLER_EMPTY    ||
        to == HANDLER_BROKEN) {
        return false;
    }

    /**
     * If this is a dormant wakeup, should be to INACTIVE.
     */
    if (from == HANDLER_DORMANT) {
        if (wakeup && to == HANDLER_INACTIVE)
            return true;
        return false;
    }

    /**
     * Now the only options left are from ACTIVE/INACTIVE to either
     * of those, or to dormant, which are all valid.
     */
    return true;
}

typedef enum {
    ONORMAL,
    OWAKEUP,
    OOVERRIDE,
} statopt;

static int set_handler_status(oem_ushort id,
                              uint8_t status,
                              uint8_t opt)
{
    int ret;
    oem_log_handler_t* handler;

    switch (status) {
    case HANDLER_EMPTY:
    case HANDLER_ACTIVE:    
    case HANDLER_INACTIVE:
    case HANDLER_DORMANT:
    case HANDLER_BROKEN:
        break;
    default:
        oem_debug_error("%d is not a valid handler status (mid %d).\n",
                        status,
                        id);
        return OEM_ERR_RANGE;
    }

    oem_log_handler_lock();

    if ((handler = get_handler_by_id(id)) == NULL) {
        oem_debug_error("No handler for MID %d found.\n",
                        id);
        ret = OEM_ERR_NOT_FOUND;
    }
    else {
        if (opt == OOVERRIDE ||
            is_valid_transfer(handler->status, status, opt == OWAKEUP)) {
            handler->status = status;
            ret = OEM_OK;
        }
        else {
            ret = OEM_ERR_INVALID;
        }
    }
    oem_log_handler_unlock();
    return ret;
}

int oem_log_handler_activate(oem_ushort id)
{
    return set_handler_status(id, HANDLER_ACTIVE, ONORMAL);
}

int oem_log_handler_deactivate(oem_ushort id)
{
    return set_handler_status(id, HANDLER_INACTIVE, ONORMAL);
}

int oem_log_handler_go_dormant(oem_ushort id)
{
    return set_handler_status(id, HANDLER_DORMANT, ONORMAL);
}

int oem_log_handler_wakeup(oem_ushort id)
{
    return set_handler_status(id, HANDLER_INACTIVE, OWAKEUP);
}

int oem_log_handler_activate_all(void)
{
    oem_log_handler_lock();
    for (oem_log_handler_t* h = log_handlers;
         h < log_handlers + OEM_LOG_HANDLER_MAX;
         ++h) {
        if (h->status == HANDLER_INACTIVE) {
            h->status = HANDLER_ACTIVE;
        }
    }
    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_handler_deactivate_all(void)
{
    oem_log_handler_lock();
    for (oem_log_handler_t* h = log_handlers;
         h < log_handlers + OEM_LOG_HANDLER_MAX;
         ++h) {
        if (h->status == HANDLER_ACTIVE) {
            h->status = HANDLER_INACTIVE;
        }
    }
    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_handler_mark_broken(oem_ushort mid)
{
    return set_handler_status(mid, HANDLER_BROKEN, OOVERRIDE);
}

int oem_log_handler_set_status(oem_ushort id,
                               uint8_t status,
                               bool override)
{
    return set_handler_status(id,
                              status,
                              override ? OOVERRIDE : ONORMAL);
}

int oem_log_handler_get_status(oem_ushort id,
                               uint8_t* status)
{
    oem_log_handler_t* h;
    if (!status) {
        return OEM_ERR_NULL;
    }

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }
    *status = h->status;

    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_add_callback(oem_ushort id,
                        oem_log_callback_t callback)
{
    oem_log_handler_t* handler;
    int ret = OEM_OK;

    if (!callback)
        return OEM_ERR_NULL;

    oem_log_handler_lock();

    if ((handler = get_handler_by_id(id)) == NULL) {
        oem_debug_error("No handler for MID %d found.\n",
                        id);
        ret = OEM_ERR_NOT_FOUND;
    }
    else {
        if (oem_list_add_back(handler->callbacks, *(void**)&callback) != OEM_OK) {
            ret = OEM_ERR_LOG_LIST;
            oem_debug_error("Failed to add callback for MID %d: callbacklist at %p\n",
                            id,
                            handler->callbacks);
        }
    }

    oem_log_handler_unlock();
    return ret;
}

int oem_log_clear_callbacks(oem_ushort mid)
{
    oem_log_handler_t* handler;
    int ret = OEM_OK;

    oem_log_handler_lock();

    if ((handler = get_handler_by_id(mid)) == NULL) {
        oem_debug_error("No handler for MID %d found.\n",
                        mid);
        ret = OEM_ERR_NOT_FOUND;
    }
    else {
        oem_list_free(handler->callbacks);
        handler->callbacks = oem_list_create();
        if (handler->callbacks == NULL) {
            ret = OEM_ERR_NOMEM;
        }
    }

    oem_log_handler_unlock();
    return ret;
}

int oem_log_get_stat(oem_ushort id,
                     oem_log_stat_t* stat)
{
    const oem_log_handler_t* h;

    if (!stat)
        return OEM_ERR_NULL;

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }

    *stat = h->stat;
    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_reset_stat(oem_ushort id)
{
    oem_log_handler_t* h;
    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }

    memset(&h->stat, 0, sizeof(h->stat));

    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_get_handler_hk(oem_ushort id,
                           oem_log_handler_hk_t* hk)
{
    const oem_log_handler_t* h;
    const oem_binary_header_t* hdr;

    if (!hk) {
        return OEM_ERR_NULL;
    }

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }

    hdr = h->recent_message;
    hk->message_id = h->message_id;
    hk->message_length = h->message_length;
    hk->recent_msg_timestamp =
        h->has_recent_message ?
        (uint32_t) hdr->week * WEEK2SEC + hdr->ms / 1000 :
        0;
    hk->callbacks = oem_list_nodes(h->callbacks);
    hk->log_count = h->stat.log_count;
    hk->log_err_count = h->stat.log_err_count;
    hk->error_cause = h->stat.error_cause;
    hk->status = h->status;
    memcpy(hk->name, h->name, OEM_LOG_HANDLER_NAME_LEN);

    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_get_recent_message(oem_ushort id,
                               void* buffer,
                               size_t offset,
                               size_t limit,
                               size_t* copied)
{
    const oem_log_handler_t* h;
    size_t to_copy;
    size_t total_message_length;

    if (!buffer)
        return OEM_ERR_NULL;

    oem_log_handler_lock();

    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }

    if (h->has_recent_message != true) {
        oem_log_handler_unlock();
        return OEM_ERR_EMPTY;
    }

    total_message_length = sizeof(oem_binary_header_t);
    total_message_length += h->message_length == OEM_LOG_HANDLER_MLEN_VARIABLE             ?
                            ((const oem_binary_header_t*)h->recent_message)->messageLength :
                            h->message_length;

    if (offset > total_message_length) {
        oem_log_handler_unlock();
        return OEM_ERR_RANGE;
    }
    
    to_copy = total_message_length - offset;
    to_copy = to_copy > limit ? limit : to_copy;

    memcpy(buffer, ((const uint8_t*)h->recent_message) + offset, to_copy);
        
    if (copied)
        *copied = to_copy;

    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_get_message_length(oem_ushort id,
                              oem_ushort* mlen)
{
    const oem_log_handler_t* h;

    if (!mlen)
        return OEM_ERR_NULL;

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }

    *mlen = h->message_length;
    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_get_handler_name(oem_ushort id, char* name)
{
    const oem_log_handler_t* h;

    if (!name)
        return OEM_ERR_NULL;

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }

    strncpy(name, h->name, OEM_LOG_HANDLER_NAME_LEN);
    name[OEM_LOG_HANDLER_NAME_LEN - 1] = '\0';

    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_ignore_missing_crc(oem_ushort id)
{
    oem_log_handler_t* h;

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }
    h->ignore_missing_crc = true;

    oem_log_handler_unlock();
    return OEM_OK;
}

int oem_log_reject_missing_crc(oem_ushort id)
{
    oem_log_handler_t* h;

    oem_log_handler_lock();
    if ((h = get_handler_by_id(id)) == NULL) {
        oem_log_handler_unlock();
        return OEM_ERR_NOT_FOUND;
    }
    h->ignore_missing_crc = false;

    oem_log_handler_unlock();
    return OEM_OK;
}

static int execute_callbacks(oem_log_handler_t* handler,
                             void* msg,
                             oem_task_context_t* ctx)
{
    void* cb;
    int ret;
    int ncb;

    /**
     * The handler mutex has been unlocked at this point.
     * If the callback deals with other handler objects, it should lock them
     * again itself.
     */

    if (!handler || !handler->callbacks || !msg || !ctx) {
        ret = OEM_ERR_NULL;
        goto early_return_callback;
    }

    ctx->cb_exec_count = 0;
    ncb =  oem_list_nodes(handler->callbacks);
    if (ncb == 0) {
        /**
         * Zero callbacks.
         */
        ret = OEM_OK;
        goto early_return_callback;
    }
    if (ncb < 0) {
        /**
         * Callback list is in invalid state.
         */
        ret = handler->stat.error_cause = OEM_ERR_UTILS_LIST_NULL;
        handler->status = HANDLER_BROKEN;
        oem_debug_error("Invalid callback list for MID %d: list at %p\n",
                        handler->message_id,
                        handler->callbacks);
        goto early_return_callback;
    }

    oem_list_tohead(handler->callbacks);
    for (int i = 0; i < ncb; ++i) {
        if ((cb = oem_list_getdata(handler->callbacks)) == NULL) {
            /**
             * Node exists but the callback is null; mark this handler broken.
             */
            ret = handler->stat.error_cause = OEM_ERR_NOT_FOUND;
            handler->status = HANDLER_BROKEN;
            goto early_return_callback;
        }
        /**
         * This ugly casting suppresses the "converting a function pointer
         * to a data pointer is not a pedantic C rule" warning.
         */
        ret = (*(oem_log_callback_t*) ((void**) &cb))(msg);
        ctx->cb_exec_count++;

        if (ret != OEM_OK) {
            oem_debug_error("Callback %d for MID %d returned error code %d.",
                            i, handler->message_id, ret);
            handler->stat.error_cause = ret;
            goto early_return_callback;
        }
        oem_list_tonext(handler->callbacks);
    }

    ret = OEM_OK;

early_return_callback:
    ctx->task_level = TASK_CALLBACK;
    return ret;
}

int oem_log_do_handle(void* msg,
                      oem_task_context_t* ctx)
{
    oem_log_handler_t* handler;
    int ret;

    if (!ctx)
        return OEM_ERR_NULL;

    oem_debug_warning("New log detected: id %d, len %d\n",
                      ctx->message_id,
                      sizeof(oem_binary_header_t) + ctx->message_length);

    oem_log_handler_lock();

    handler = get_handler_by_id(ctx->message_id);
    if (handler == NULL) {
        oem_debug_error("No handler for MID %d: discarding msg.\n",
                        ctx->message_id);
        ret = OEM_ERR_LOG_STRAY;
        goto early_return_log;
    }

    /**
     * Completely ignore the message if the handler is dormant or broken.
     */
    if (handler->status == HANDLER_DORMANT ||
        handler->status == HANDLER_BROKEN) {
        ret = OEM_OK;
        oem_debug_warning("Skipping dormant/broken handler '%s' at MID %d\n",
                          handler->name,
                          handler->message_id);
        goto early_return_log;
    }
    
    handler->stat.log_count++;

    /**
     * Check if the message length fits this handler.
     * Note that the handler's message length includes the header (misnomer).
     * Zero messageLength means the size is variable (e.g., "per GPS sat").
     */
    size_t total_length = sizeof(oem_binary_header_t) + ctx->message_length;
    if (handler->message_length > 0 &&
        handler->message_length != total_length) {
        oem_debug_error("Invalid mlen for MID %d: got %d, expected %d.\n",
                        ctx->message_id,
                        total_length,
                        handler->message_length);
        handler->stat.log_err_count++;
        ret = handler->stat.error_cause = OEM_ERR_LOG_BODY_SIZE;
        goto early_return_log;  
    }

    if (ctx->crc_read_skipped && handler->ignore_missing_crc == false) {
        /* CRC was not read */
        ret = handler->stat.error_cause = OEM_ERR_LOG_MISSING_CRC;
        goto early_return_log;
    }
    else
        oem_debug_warning("Log %d CRC verification skipped: handler %s\n",
                          handler->message_id, handler->name);

    /** 
     * Store the recent message.
     */
    if (handler->recent_message == NULL) {
        ret = handler->stat.error_cause = OEM_ERR_NOBUF;
        handler->status = HANDLER_BROKEN;
        oem_debug_error("NULL message buffer for handler '%s' at MID %d! "
                   "Handler will be marked broken.\n",
                   handler->name,
                   handler->message_id);
        goto early_return_log;
    }

    if (handler->message_length == 0 &&
        total_length > OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE) {
        oem_debug_warning("Too long message size truncated (%d -> %d). MID %d\n",
                          total_length, OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE,
                          handler->message_id);
        total_length = OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE;
    }

    memcpy(handler->recent_message, msg, total_length);
    handler->has_recent_message = true;

    if (handler->status == HANDLER_INACTIVE) {
        ret = handler->stat.error_cause = OEM_OK;
        oem_debug_warning("Skipping inactive handler '%s' at MID %d\n",
                          handler->name,
                          handler->message_id);
        goto early_return_log;
    }

    handler->stat.error_cause = OEM_OK;

    oem_log_handler_unlock();
    return execute_callbacks(handler, msg, ctx);

early_return_log:
    oem_log_handler_unlock();
    ctx->task_level = TASK_LOG;
    return ret;
}
