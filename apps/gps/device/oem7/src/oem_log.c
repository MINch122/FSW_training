/**
 * @file oem_log.c
 * @brief OEM7 log handler.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#if OEM_DEBUG
#include <stdio.h>`
#endif

#include "oem_log.h"

#include "oem_task.h"  /* oem_task_context_t */
#include "oem_utils.h" /* mutex */

#define WEEK2SEC 604800U

struct oem_log_handler_s {
    char                  name[OEM_LOG_HANDLER_NAME_LEN];
    oem_callbacklist      callbacks;
    void*                 recentMessage;
    oem_log_handler_stat_t  stat;
    oem_ushort            messageId;
    oem_ushort            messageLength;
    uint8_t               status;
    bool                  ignoreChecksum;
};

/**
 * Future design requirements:
 * If the OBC received logs from more than one ports, the log handler should
 * either 1) have expanded slots for each port (logHandler[NUMPORTS][MAX]),
 * or have a dedicated mutex individually.
 */
static oem_log_handler_t logHandler[OEM_LOG_HANDLER_MAX];

static pthread_mutex_t hmut;

int OEM_Log_HandlerInit(void)
{
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr)         != 0 ||
        pthread_mutexattr_setprotocol(&attr,
                        PTHREAD_PRIO_INHERIT) != 0 ||
        pthread_mutexattr_settype(&attr,
                    PTHREAD_MUTEX_RECURSIVE)  != 0 ||
        pthread_mutex_init(&hmut,
                           &attr)             != 0) {
        return OEM_ERR_MUTEX_INIT;
    }
    return OEM_OK;
}

int OEM_Log_HandlerLock(void)
{
    return pthread_mutex_lock(&hmut) == 0 ?
           OEM_OK :
           OEM_ERR_MUTEX_LOCK;
}

int OEM_Log_HandlerUnlock(void)
{
    return pthread_mutex_unlock(&hmut) == 0 ?
           OEM_OK :
           OEM_ERR_MUTEX_UNLOCK;
}

static oem_log_handler_t* LogHandlerGetEmptySlot(void)
{
    for (int i = 0; i < OEM_LOG_HANDLER_MAX; ++i) {
        if (logHandler[i].status == HANDLER_EMPTY) {
            return &logHandler[i];
        }
    }
    return NULL;
}

static oem_log_handler_t* GetHandlerInternal(oem_ushort id)
{
    for (int i = 0; i < OEM_LOG_HANDLER_MAX; ++i) {
        if (logHandler[i].messageId == id)
            return &logHandler[i];
    }
    return NULL;
}

const oem_log_handler_t* OEM_Log_GetHandler(oem_ushort id)
{
    const oem_log_handler_t* handler;
    OEM_Log_HandlerLock();
    handler = GetHandlerInternal(id);
    OEM_Log_HandlerUnlock();
    return handler;
}

static void LogHandlerPurge(oem_log_handler_t* handler)
{
    if (!handler)
        return;
    if (handler->recentMessage) {
        free(handler->recentMessage);
    }
    if (handler->callbacks) {
        oem_list_free(handler->callbacks);
    }
    memset(handler, 0, sizeof(*handler));
    handler->status = HANDLER_EMPTY;
}

int OEM_Log_RegisterHandler(const char* name,
                            oem_ushort id,
                            oem_ushort mlen)
{
    oem_log_handler_t* newHandler = NULL;
    int ret = OEM_OK;

    OEM_Log_HandlerLock();

    if (GetHandlerInternal(id)) {
        DebugError("Handler for MID %d has already been registered.\n",
                   id);
        ret = OEM_ERR_EXISTS;
    }
    else if ((newHandler = LogHandlerGetEmptySlot()) == NULL) {
        DebugError("Handler queue is full: mid %d.\n",
                   id);
        ret = OEM_ERR_FULL;
    }
    else if ((newHandler->callbacks = oem_list_create()) == NULL) {
        ret = OEM_ERR_NOMEM;
    }
    else if (
        (newHandler->recentMessage = malloc(mlen > 0 ?
                                            mlen :
                                            OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE)
        ) == NULL
    ) {
        DebugError("malloc failed for handler ID %d.\n",
                    id);
        mlk_list_free(newHandler->callbacks);
        ret = OEM_ERR_NOMEM;
    }
    else {
        strncpy(newHandler->name, name, sizeof(newHandler->name));
        newHandler->name[sizeof(newHandler->name) - 1] = '\0';
        memset(&newHandler->stat, 0, sizeof(newHandler->stat));
        newHandler->messageId = id;
        newHandler->messageLength = mlen;
        newHandler->status = HANDLER_INACTIVE;
        newHandler->ignoreChecksum = false;
    }

    OEM_Log_HandlerUnlock();
    return ret;

}

int OEM_Log_UnregisterHandler(oem_ushort id)
{

    oem_log_handler_t* handler;
    int ret = OEM_OK;

    OEM_Log_HandlerLock();

    if ((handler = GetHandlerInternal(id)) == NULL) {
        DebugError("No handler for MID %d found.\n",
                  id);
        ret = OEM_ERR_NOTFOUND;
    }
    else {
        LogHandlerPurge(handler);
    }

    OEM_Log_HandlerUnlock();
    return ret;

}

int OEM_Log_AddCallback(oem_ushort id,
                        oem_log_callback_t callback)
{
    oem_log_handler_t* handler;
    int ret = OEM_OK;

    OEM_Log_HandlerLock();

    if ((handler = GetHandlerInternal(id)) == NULL) {
        DebugError("No handler for MID %d found.\n",
                  id);
        ret = OEM_ERR_NOTFOUND;
    }
    else {
        if (oem_list_add_back(handler->callbacks, *(void**)&callback) != M_SUCCESS) {
            ret = OEM_ERR_LIST;
            DebugError("Failed to add callback for MID %d: callbacklist at %p\n",
                        id,
                        handler->callbacks);
        }
    }

    OEM_Log_HandlerUnlock();
    return ret;
}

int OEM_Log_ClearCallbacks(oem_ushort mid)
{
    oem_log_handler_t* handler;
    int ret = OEM_OK;

    OEM_Log_HandlerLock();

    if ((handler = GetHandlerInternal(mid)) == NULL) {
        DebugError("No handler for MID %d found.\n",
                  mid);
        ret = OEM_ERR_NOTFOUND;
    }
    else {
        oem_list_free(handler->callbacks);
        handler->callbacks = oem_list_create();
        if (handler->callbacks == NULL) {
            ret = OEM_ERR_NOMEM;
        }
    }

    OEM_Log_HandlerUnlock();
    return ret;
}

static bool IsValidStatusTransfer(uint8_t from, uint8_t to, bool wakeup)
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
     * Now the only options left are form ACTIVE/INACTIVE to either
     * of those, or to dormant, which are all valid.
     */
    return true;
}

typedef enum {
    STAT_NORMAL,
    STAT_WAKEUP,
    STAT_OVERRIDE,
} statopt;

static int SetHandlerStatusInternal(oem_ushort id,
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
        DebugError("%d is not a valid handler status (mid %d).\n",
                   status,
                   id);
        return OEM_ERR_RANGE;
    }

    OEM_Log_HandlerLock();

    if ((handler = GetHandlerInternal(id)) == NULL) {
        DebugError("No handler for MID %d found.\n",
                   id);
        ret = OEM_ERR_NOTFOUND;
    }
    else {
        if (opt == STAT_OVERRIDE ||
            IsValidStatusTransfer(handler->status, status, opt == STAT_WAKEUP)) {
            handler->status = status;
            ret = OEM_OK;
        }
        else {
            ret = OEM_ERR_INVALID;
        }
    }
    OEM_Log_HandlerUnlock();
    return ret;
}

int OEM_Log_SetHandlerStatus(oem_ushort id,
                             uint8_t status,
                             bool override)
{
    return SetHandlerStatusInternal(id,
                                    status,
                                    override ? STAT_OVERRIDE : STAT_NORMAL);
}

int OEM_Log_GetHandlerStatus(oem_ushort id,
                             uint8_t* status)
{
    oem_log_handler_t* h;
    if (!status) {
        return OEM_ERR_NULL;
    }

    OEM_Log_HandlerLock();
    if ((h = GetHandlerInternal(id)) == NULL) {
        return OEM_ERR_NOTFOUND;
    }
    *status = h->status;

    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

int OEM_Log_HandlerActivate(oem_ushort id)
{
    return SetHandlerStatusInternal(id, HANDLER_ACTIVE, STAT_NORMAL);
}

int OEM_Log_HandlerDeacivate(oem_ushort id)
{
    return SetHandlerStatusInternal(id, HANDLER_INACTIVE, STAT_NORMAL);
}

int OEM_Log_HandlerGoDormant(oem_ushort id)
{
    return SetHandlerStatusInternal(id, HANDLER_DORMANT, STAT_NORMAL);
}

int OEM_Log_HandlerWakeup(oem_ushort id)
{
    return SetHandlerStatusInternal(id, HANDLER_INACTIVE, STAT_WAKEUP);
}

int OEM_Log_HandlerActivateAll(void)
{
    OEM_Log_HandlerLock();
    for (oem_log_handler_t* h = logHandler;
         h < logHandler + OEM_LOG_HANDLER_MAX;
         ++h) {
        if (h->status == HANDLER_INACTIVE) {
            h->status = HANDLER_ACTIVE;
        }
    }
    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

int OEM_Log_HandlerDeactivateAll(void)
{
    OEM_Log_HandlerLock();
    for (oem_log_handler_t* h = logHandler;
         h < logHandler + OEM_LOG_HANDLER_MAX;
         ++h) {
        if (h->status == HANDLER_ACTIVE) {
            h->status = HANDLER_INACTIVE;
        }
    }
    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

int OEM_Log_HandlerSetBroken(oem_ushort mid)
{
    return SetHandlerStatusInternal(mid, HANDLER_BROKEN, STAT_OVERRIDE);
}

static int GetHandlerAttributeDirect(const oem_log_handler_t* handler,
                                     void* attr,
                                     size_t offset,
                                     size_t len)
{
    if (!handler || !attr) {
        return OEM_ERR_NULL;
    }
    if (len + offset > sizeof(*handler)) {
        return OEM_ERR_RANGE;
    }
    memcpy(attr, ((const uint8_t*) handler) + offset, len);
    return OEM_OK;
}

static int GetHandlerAttribute(oem_ushort id,
                               void* attr,
                               size_t offset,
                               size_t len)
{
    const oem_log_handler_t* handler;
    int ret;

    if (!attr) {
        return OEM_ERR_NULL;
    }

    OEM_Log_HandlerLock();

    if ((handler = GetHandlerInternal(id)) == NULL) {
        OEM_Log_HandlerUnlock();
        return OEM_ERR_NOTFOUND;
    }

    ret = GetHandlerAttributeDirect(handler, attr, offset, len);

    OEM_Log_HandlerUnlock();
    return ret;
}

int OEM_Log_GetMessageLength(oem_ushort id,
                             oem_ushort* mlen)
{
    return GetHandlerAttribute(id,
                               mlen,
                               offsetof(oem_log_handler_t, messageLength),
                               sizeof(*mlen));
}

int OEM_Log_GetMessageStatistics(oem_ushort id,
                                 oem_log_handler_stat_t* stat)
{
    return GetHandlerAttribute(id,
                               stat,
                               offsetof(oem_log_handler_t, stat),
                               sizeof(*stat));
}

int OEM_Log_GetHandlerName(oem_ushort id, char* name)
{
    return GetHandlerAttribute(id,
                               name,
                               offsetof(oem_log_handler_t, name),
                               OEM_LOG_HANDLER_NAME_LEN);
}

int OEM_Log_ResetHandlerCounters(oem_ushort id)
{
    oem_log_handler_t* h;
    OEM_Log_HandlerLock();
    if ((h = GetHandlerInternal(id)) == NULL) {
        OEM_Log_HandlerUnlock();
        return OEM_ERR_NOTFOUND;
    }

    memset(&h->stat, 0, sizeof(h->stat));

    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

int OEM_Log_GethandlerHousekeeping(oem_ushort id,
                                   oem_log_handler_hk_t* hk)
{
    const oem_log_handler_t* h;
    const oem_binary_header_t* hdr;

    if (!hk) {
        return OEM_ERR_NULL;
    }

    OEM_Log_HandlerLock();
    if ((h = GetHandlerInternal(id)) == NULL) {
        OEM_Log_HandlerUnlock();
        return OEM_ERR_NOTFOUND;
    }

    hdr = h->recentMessage;
    hk->messageId = h->messageId;
    hk->messageLength = h->messageLength;
    hk->recentMsgTimeStamp =
        OEM_Task_IsSynced(h->recentMessage)              ?
        (uint32_t) hdr->week * WEEK2SEC + hdr->ms / 1000 :
        0;
    hk->attachedCallbacks = oem_list_nodes(h->callbacks);
    hk->logCount = h->stat.logCount;
    hk->logErrCount = h->stat.logErrCount;
    hk->errorCause = h->stat.errorCause;
    hk->status = h->status;
    hk->ignoreChecksum = h->ignoreChecksum;
    memcpy(hk->name, h->name, OEM_LOG_HANDLER_NAME_LEN);

    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

int OEM_Log_DumpRecentMessage(oem_ushort id,
                              void* buffer,
                              size_t bufsize,
                              size_t* copiedSize)
{
    const oem_log_handler_t* h;
    const oem_binary_header_t* hdr;
    int ret;

    if (!buffer) {
        return OEM_ERR_NULL;
    }

    OEM_Log_HandlerLock();

    if ((h = GetHandlerInternal(id)) == NULL) {
        ret = OEM_ERR_NOTFOUND;
    }
    else if (OEM_Task_IsSynced(h->recentMessage) != true) {
        ret = OEM_ERR_EMPTY;
    }
    else {
        if (h->messageLength > 0) {
            bufsize = bufsize > h->messageLength ? h->messageLength : bufsize;
        }
        else {
            hdr = h->recentMessage;
            bufsize = bufsize > sizeof(*hdr) + hdr->messageLength ?
                      sizeof(*hdr) + hdr->messageLength :
                      bufsize;
        }

        if (copiedSize) {
            *copiedSize = bufsize;
        }

        memcpy(buffer, h->recentMessage, bufsize);
        ret = OEM_OK;
    }

    OEM_Log_HandlerUnlock();
    return ret;
}

int OEM_Log_DisableCsVerification(oem_ushort id)
{
    oem_log_handler_t* h;

    OEM_Log_HandlerLock();
    if ((h = GetHandlerInternal(id)) == NULL) {
        OEM_Log_HandlerUnlock();
        return OEM_ERR_NOTFOUND;
    }
    h->ignoreChecksum = true;

    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

int OEM_Log_EnableCsVerification(oem_ushort id)
{
    oem_log_handler_t* h;

    OEM_Log_HandlerLock();
    if ((h = GetHandlerInternal(id)) == NULL) {
        OEM_Log_HandlerUnlock();
        return OEM_ERR_NOTFOUND;
    }
    h->ignoreChecksum = false;

    OEM_Log_HandlerUnlock();
    return OEM_OK;
}

static int ExecuteHandlerCallback(oem_log_handler_t* handler,
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

    if (!handler || !handler->callbacks || !msg) {
        ret = OEM_ERR_NULL;
    }

    ncb =  oem_list_nodes(handler->callbacks);
    if (ncb == 0) {
        /**
         * Zero callbacks.
         */
        ret = OEM_ERR_EMPTY;
        goto early_return_callback;
    }

    oem_list_tohead(handler->callbacks);
    for (int i = 0; i < ncb; ++i) {
        if ((cb = oem_list_getdata(handler->callbacks)) == NULL) {
            /**
             * Node exists but the callback is null; mark this handler broken.
             */
            ret = handler->stat.errorCause = OEM_ERR_NOTFOUND;
            handler->status = HANDLER_BROKEN;
            goto early_return_callback;
        }
        /**
         * This ugly casting suppresses the "converting a function pointer
         * to a data pointer is not a pedantic C rule" warning.
         */
        ret = (*(oem_log_callback_t*) ((void**) &cb))(msg);
        if (ctx) {
            ctx->callbackExecCnt++;
        }
        oem_list_tonext(handler->callbacks);
    }

    ret = OEM_OK;

early_return_callback:
    if (ctx) {
        ctx->taskLevel = TASK_CALLBACK;
    }
    return ret;
}

int _DoLogHandle(void* msg,
                 oem_task_context_t* ctx)
{
    oem_log_handler_t* handler;
    const oem_binary_header_t* header = msg;
    oem_crc crcAtSite;
    oem_ushort mlen;
    int ret;

    if (!ctx)
        return OEM_ERR_NULL;

    DebugInfo("New log detected: id %d, len %d\n",
              header->messageID,
              sizeof(*header) + header->messageLength);

    OEM_Log_HandlerLock();

    handler = GetHandlerInternal(header->messageID);
    if (handler == NULL) {
        DebugError("No handler for MID %d: discarding msg.\n",
                   header->messageID);
        ret = OEM_ERR_STRAY;
        goto early_return_log;
    }

    /**
     * Completely ignore the message if the handler is dormant or broken.
     */
    if (handler->status == HANDLER_DORMANT ||
        handler->status == HANDLER_BROKEN) {
        ret = OEM_OK;
        DebugWarning("Skipping dormant/broken handler '%s' at MID %d\n",
                     handler->name,
                     handler->messageId);
        goto early_return_log;
    }

    handler->stat.logCount++;

    /**
     * Check if the message length fits this handler.
     * Note that the handler's message length includes the header (misnomer).
     * Zero messageLength means the size is variable (e.g., "per GPS sat").
     */
    mlen = sizeof(*header) + header->messageLength;
    if (handler->messageLength > 0 &&
        handler->messageLength != mlen) {
        DebugError("Invalid mlen for MID %d: got %d, expected %d.\n",
                  header->messageID,
                  mlen,
                  handler->messageLength);
        handler->stat.logErrCount++;
        ret = handler->stat.errorCause = OEM_ERR_LEN_MSG;
        goto early_return_log;
    }

    if (handler->ignoreChecksum == false) {
        if (ctx->crcReadSkipped) {
            ret = handler->stat.errorCause = OEM_ERR_READ_CRC;
            goto early_return_log;
        }
        crcAtSite = OEM_CalculateBlockCRC32(msg, mlen);
        if (ctx->msgCrc != crcAtSite) {
            DebugError("crc verification failed: MID %d, "
                       "expected %08X, got %08X.\n",
                        header->messageID,
                        crcAtSite, ctx->msgCrc);
            handler->stat.logErrCount++;
            ret = handler->stat.errorCause = OEM_ERR_CRC;
            goto early_return_log;
        }
        DebugInfo("Log message CRC verification passed: crc 0x%08X\n", ctx->msgCrc);
    }
    else
        Debug("Log %d CRC verification skipped: handler %s\n",
              handler->messageId, handler->name);

    /**
     * Store the recent message.
     */
    if (handler->recentMessage == NULL) {
        ret = handler->stat.errorCause = OEM_ERR_NOBUF;
        handler->status = HANDLER_BROKEN;
        DebugError("NULL message buffer for handler '%s' at MID %d! "
                   "Handler will be marked broken.\n",
                   handler->name,
                   handler->messageId);
        goto early_return_log;
    }

    if (handler->messageLength == 0 &&
        mlen > OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE) {
        DebugWarning("Too long message size truncated (%d -> %d). MID %d\n",
                     mlen, OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE,
                     handler->messageId);
        mlen = OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE;
    }

    memcpy(handler->recentMessage, msg, mlen);

    if (handler->status == HANDLER_INACTIVE) {
        ret = handler->stat.errorCause = OEM_OK;
        DebugWarning("Skipping inactive handler '%s' at MID %d\n",
                     handler->name,
                     handler->messageId);
        goto early_return_log;
    }

    handler->stat.errorCause = OEM_OK;

    OEM_Log_HandlerUnlock();
    return ExecuteHandlerCallback(handler, msg, ctx);

early_return_log:
    OEM_Log_HandlerUnlock();
    ctx->taskLevel = TASK_LOG;
    return ret;
}
