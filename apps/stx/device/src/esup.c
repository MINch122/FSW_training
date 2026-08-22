/**
 * @file esup.c
 * @brief Implementation of the ESUP engine.
 *
 * Concurrency in one paragraph: esup_start_session sends the command
 * synchronously and arms the session; the driver is a plain repeated tick that
 * reads the wire, routes replies into session mailboxes, and advances session
 * timers. One mutex guards the session pool and the transmit path (command sends
 * from start_session, poll/ACK sends from the driver); the blocking read runs
 * outside it, so start_session and wait never stall behind link I/O. esup_wait is
 * a timed wait on the engine's completion broadcast.
 *
 * Note on the synchronous send: a start_session may transmit while the module is
 * answering another session (half-duplex). A clashed frame is simply lost and
 * both sides recover through the normal retry/poll machinery.
 *
 * The engine struct is private to this file: callers hold an opaque
 * esup_engine_t* from esup_engine_create. The session pool is embedded by value
 * (esup_session_priv.h), so no per-session heap allocation happens.
 */
#define _POSIX_C_SOURCE 200809L

#include "esup.h"
#include "esup_session_priv.h"
#include "esup_utils.h"

#include <pthread.h>
#include <stdlib.h>
#include <time.h>

/** Upper bound on how long one tick may block waiting for input. */
#define ESUP_TICK_MAX_BLOCK_MS 100u

/** Slice for esup_wait's condition sleep; bounds deadline check granularity. */
#define ESUP_WAIT_SLICE_MS     20u

/**
 * Extra time esup_transact waits past a session's own deadline, so a session-side
 * timeout surfaces as its real result (EXPIRED/UNREACHABLE/...) rather than
 * PENDING.
 */
#define ESUP_TRANSACT_WAIT_MARGIN_MS 1000u

/**
 * @brief The transaction engine bound to one module over one link.
 */
struct esup_engine {
    esup_framer_t   framer;                     /**< Link layer (owns the wire). */
    uint16_t        module_id;                  /**< Target unit (last 4 hex of serial). */
    esup_session_t  sessions[ESUP_SESSION_MAX]; /**< Session pool. */
    pthread_mutex_t lock;      /**< Guards sessions and the transmit path. */
    pthread_cond_t  done_cond; /**< Broadcast on completions, for esup_wait. */
    pthread_cond_t  wake_cond; /**< Wakes an idle driver on start_session or stop. */
    pthread_t       thread;    /**< Thread created by esup_start (if threaded). */
    bool            threaded;  /**< esup_start created the driver thread. */
    bool            running;   /**< A driver loop is active. */
    bool            stop;      /**< Stop request for the driver loop. */
};

/* ------------------------------------------------------------------------- */
/* Time helpers                                                              */
/* ------------------------------------------------------------------------- */

/**
 * @brief Read the engine's monotonic clock (via the link).
 *
 * @param engine Engine.
 * @return Milliseconds from an arbitrary fixed origin.
 */
static uint32_t now_ms(esup_engine_t* engine)
{
    return esup_framer_now(&engine->framer);
}

/**
 * @brief Return the earlier of two absolute monotonic deadlines (wrap-safe).
 *
 * @param a First deadline.
 * @param b Second deadline.
 * @return Whichever comes first.
 */
static uint32_t sooner(uint32_t a, uint32_t b)
{
    if ((int32_t)(a - b) < 0)
        return a;
    return b;
}

/**
 * @brief Whether an absolute engine-clock deadline has been reached (wrap-safe).
 *
 * @param engine   Engine.
 * @param deadline Absolute deadline.
 * @return true if reached.
 */
static bool reached(esup_engine_t* engine, uint32_t deadline)
{
    return (int32_t)(deadline - now_ms(engine)) <= 0;
}

/**
 * @brief Absolute CLOCK_MONOTONIC timespec @p ms from now, for cond_timedwait.
 *
 * @param ms Delay in milliseconds.
 * @param ts Out: absolute timespec.
 */
static void timespec_in(uint32_t ms, struct timespec* ts)
{
    clock_gettime(CLOCK_MONOTONIC, ts);
    ts->tv_sec += (time_t)(ms / 1000u);
    ts->tv_nsec += (long)(ms % 1000u) * 1000000L;
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_sec += 1;
        ts->tv_nsec -= 1000000000L;
    }
}

/* ------------------------------------------------------------------------- */
/* Pool and routing (call with the engine lock held)                         */
/* ------------------------------------------------------------------------- */

/**
 * @brief Claim a session slot for a selector, enforcing selector uniqueness.
 *
 * @param engine  Engine (locked).
 * @param command Command selector.
 * @param type    Type selector.
 * @param out     Out: the claimed session on success, NULL otherwise.
 * @return ESUP_OK on success; ESUP_SESSION_DUP or ESUP_SESSION_FULL otherwise.
 */
static esup_ret_t session_alloc(esup_engine_t* engine, uint16_t command, uint16_t type,
                         esup_session_t** out)
{
    *out = NULL;
    for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
        esup_session_t* s = &engine->sessions[i];

        if (s->in_use && s->command == command && s->type == type)
            return ESUP_SESSION_DUP;
    }
    for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
        esup_session_t* s = &engine->sessions[i];

        if (!s->in_use) {
            s->in_use = true;
            s->command = command;
            s->type = type;
            *out = s;
            return ESUP_OK;
        }
    }
    return ESUP_SESSION_FULL;
}

/**
 * @brief Route every decoded frame to its owning session's mailbox.
 *
 * Frames for another unit, or with no awaiting owner, are dropped.
 *
 * @param engine Engine (locked).
 */
static void route_frames(esup_engine_t* engine)
{
    esup_frame_t frame;

    while (esup_framer_pop(&engine->framer, &frame) == 1) {
        if (frame.module_id != engine->module_id)
            continue;
        for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
            esup_session_t* s = &engine->sessions[i];

            if (esup_session_awaiting(s, frame.command, frame.type)) {
                esup_session_deposit(s, &frame);
                break;
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Lifecycle                                                                 */
/* ------------------------------------------------------------------------- */

esup_engine_t* esup_engine_create(const esup_wire_t* wire, uint16_t module_id)
{
    esup_engine_t* engine = malloc(sizeof(*engine));

    if (engine == NULL) {
        ESUP_LOGE("engine allocation failed");
        return NULL;
    }
    if (esup_framer_init(&engine->framer, wire) != ESUP_OK) {
        ESUP_LOGE("framer init failed");
        goto fail_free;
    }

    engine->module_id = module_id;
    for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
        engine->sessions[i].in_use = false;
        engine->sessions[i].state = ESUP_SESSION_FREE;
    }
    engine->threaded = false;
    engine->running = false;
    engine->stop = false;

    if (pthread_mutex_init(&engine->lock, NULL) != 0) {
        ESUP_LOGE("mutex init failed");
        goto fail_free;
    }
    if (pthread_cond_init(&engine->wake_cond, NULL) != 0) {
        ESUP_LOGE("wake_cond init failed");
        goto fail_mutex;
    }

    /* done_cond is timed-waited against CLOCK_MONOTONIC. */
    pthread_condattr_t attr;

    if (pthread_condattr_init(&attr) != 0) {
        ESUP_LOGE("condattr init failed");
        goto fail_wake;
    }
    if (pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) != 0) {
        ESUP_LOGE("condattr setclock failed");
        pthread_condattr_destroy(&attr);
        goto fail_wake;
    }
    if (pthread_cond_init(&engine->done_cond, &attr) != 0) {
        ESUP_LOGE("done_cond init failed");
        pthread_condattr_destroy(&attr);
        goto fail_wake;
    }
    pthread_condattr_destroy(&attr);
    return engine;

fail_wake:
    pthread_cond_destroy(&engine->wake_cond);
fail_mutex:
    pthread_mutex_destroy(&engine->lock);
fail_free:
    free(engine);
    return NULL;
}

void esup_engine_destroy(esup_engine_t* engine)
{
    if (engine == NULL)
        return;
    if (engine->threaded)
        (void)esup_stop(engine);
    pthread_cond_destroy(&engine->done_cond);
    pthread_cond_destroy(&engine->wake_cond);
    pthread_mutex_destroy(&engine->lock);
    free(engine);
}

esup_ret_t esup_engine_reset(esup_engine_t* engine)
{
    if (engine == NULL) {
        ESUP_LOGE("engine is NULL");
        return ESUP_SESSION_ABORTED;
    }

    pthread_mutex_lock(&engine->lock);
    if (engine->running) {
        pthread_mutex_unlock(&engine->lock);
        ESUP_LOGW("refused: driver is running");
        return ESUP_SESSION_ABORTED;
    }
    for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
        engine->sessions[i].in_use = false;
        engine->sessions[i].state = ESUP_SESSION_FREE;
    }
    esup_framer_reset(&engine->framer);
    pthread_mutex_unlock(&engine->lock);
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* Command submission                                                        */
/* ------------------------------------------------------------------------- */

esup_ret_t esup_start_session(esup_engine_t* engine, uint16_t command, uint16_t type,
                       const void* req_data, uint16_t req_len,
                       esup_result_t* result, uint32_t timeout_ms,
                       esup_session_t** out_session)
{
    if (out_session != NULL)
        *out_session = NULL;

    if (engine == NULL || out_session == NULL || (req_len > 0 && req_data == NULL)
        || req_len > ESUP_DATA_MAX) {
        ESUP_LOGE("bad argument (engine=%p out=%p req_len=%u req_data=%p)",
                  (void*)engine, (void*)out_session, req_len, req_data);
        return ESUP_SESSION_ABORTED;
    }

    pthread_mutex_lock(&engine->lock);

    esup_session_t* s = NULL;
    esup_ret_t rc = session_alloc(engine, command, type, &s);

    if (rc == ESUP_OK) {
        esup_session_init(s, req_data, req_len, result, now_ms(engine),
                          timeout_ms);
        /* Fire the command right now (one step performs the pending send); the
         * session is then waiting for its reply and the driver takes over. */
        esup_session_step(s, &engine->framer, engine->module_id, now_ms(engine));
        pthread_cond_signal(&engine->wake_cond);
        *out_session = s;
    }
    pthread_mutex_unlock(&engine->lock);

    if (rc != ESUP_OK)
        ESUP_LOGW("no session for cmd=0x%04X type=0x%04X (ret 0x%08X)", command,
                  type, (unsigned)rc);
    return rc;
}

esup_ret_t esup_transact(esup_engine_t* engine, uint16_t command, uint16_t type,
                  const void* req_data, uint16_t req_len, esup_result_t* result,
                  uint32_t timeout_ms)
{
    esup_session_t* session = NULL;
    esup_ret_t rc = esup_start_session(engine, command, type, req_data, req_len,
                                       result, timeout_ms, &session);

    if (rc != ESUP_OK)
        return rc; /* the specific refusal (FULL / DUP / ABORTED) passes through */

    rc = esup_wait(engine, session, timeout_ms + ESUP_TRANSACT_WAIT_MARGIN_MS);

    /* esup_transact owns the whole lifecycle. If the wait gave up (PENDING), the
     * session is still live and still references the caller's borrowed buffers,
     * so it MUST be cancelled before returning (never leave it to write into a
     * returned stack frame). Then reap the slot. All under the lock so this is
     * mutually exclusive with the driver. */
    pthread_mutex_lock(&engine->lock);
    if (rc == ESUP_SESSION_PENDING) {
        esup_session_cancel(session);
        rc = esup_session_result(session);
    }
    esup_session_reap(session);
    pthread_mutex_unlock(&engine->lock);
    return rc;
}

void esup_reap(esup_engine_t* engine, esup_session_t* session)
{
    if (engine == NULL || session == NULL)
        return;
    pthread_mutex_lock(&engine->lock);
    esup_session_reap(session);
    pthread_mutex_unlock(&engine->lock);
}

esup_ret_t esup_get(esup_engine_t* engine, uint16_t type, esup_result_t* result,
             uint32_t timeout_ms)
{
    return esup_transact(engine, ESUP_CMD_GET, type, NULL, 0, result, timeout_ms);
}

esup_ret_t esup_set(esup_engine_t* engine, uint16_t type, const void* data,
             uint16_t len, esup_result_t* result, uint32_t timeout_ms)
{
    return esup_transact(engine, ESUP_CMD_SET, type, data, len, result,
                         timeout_ms);
}

/* ------------------------------------------------------------------------- */
/* Driver                                                                    */
/* ------------------------------------------------------------------------- */

int esup_tick(esup_engine_t* engine)
{
    if (engine == NULL) {
        ESUP_LOGE("engine is NULL");
        return -1; /* this returns an active-session count, not an esup_ret_t */
    }

    pthread_mutex_lock(&engine->lock);

    uint32_t now = now_ms(engine);

    for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
        if (esup_session_is_live(&engine->sessions[i]))
            esup_session_step(&engine->sessions[i], &engine->framer,
                              engine->module_id, now);
    }

    /* Terminations happened in the steps above; let waiters re-check. */
    pthread_cond_broadcast(&engine->done_cond);

    uint32_t wake = now_ms(engine) + ESUP_TICK_MAX_BLOCK_MS;
    int active = 0;

    /* Only LIVE sessions schedule work; a terminated-but-unreaped session is
     * occupied but must not be stepped or drag the wake time to "now". */
    for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
        esup_session_t* s = &engine->sessions[i];

        if (!esup_session_is_live(s))
            continue;
        active++;
        wake = sooner(wake, s->next_action_ms);
        wake = sooner(wake, s->deadline_ms);
    }
    pthread_mutex_unlock(&engine->lock);

    if (active == 0)
        return 0;

    /* Blocking read outside the lock: start_session/wait stay responsive. */
    (void)esup_framer_recv(&engine->framer, wake);

    pthread_mutex_lock(&engine->lock);
    route_frames(engine);
    pthread_mutex_unlock(&engine->lock);
    return active;
}

esup_ret_t esup_run(esup_engine_t* engine)
{
    if (engine == NULL) {
        ESUP_LOGE("engine is NULL");
        return ESUP_SESSION_ABORTED;
    }

    pthread_mutex_lock(&engine->lock);
    if (engine->running) {
        pthread_mutex_unlock(&engine->lock);
        ESUP_LOGW("refused: a driver is already running");
        return ESUP_SESSION_ABORTED;
    }
    engine->running = true;

    while (!engine->stop) {
        bool active = false;

        for (unsigned i = 0; i < ESUP_SESSION_MAX; i++) {
            if (esup_session_is_live(&engine->sessions[i])) {
                active = true;
                break;
            }
        }
        if (!active) {
            /* Idle: sleep until esup_start_session or esup_stop wakes us. */
            pthread_cond_wait(&engine->wake_cond, &engine->lock);
            continue;
        }
        pthread_mutex_unlock(&engine->lock);
        (void)esup_tick(engine);
        pthread_mutex_lock(&engine->lock);
    }
    engine->running = false;
    pthread_mutex_unlock(&engine->lock);

    /* Wake any waiters so they observe the driver is gone. */
    pthread_cond_broadcast(&engine->done_cond);
    return ESUP_OK;
}

/**
 * @brief pthread entry wrapping esup_run for esup_start.
 *
 * @param arg The engine.
 * @return NULL.
 */
static void* run_thread_main(void* arg)
{
    (void)esup_run(arg);
    return NULL;
}

esup_ret_t esup_start(esup_engine_t* engine)
{
    if (engine == NULL) {
        ESUP_LOGE("engine is NULL");
        return ESUP_SESSION_ABORTED;
    }

    pthread_mutex_lock(&engine->lock);
    if (engine->running || engine->threaded) {
        pthread_mutex_unlock(&engine->lock);
        ESUP_LOGW("refused: a driver is already running");
        return ESUP_SESSION_ABORTED;
    }
    /* Claim the driver slot BEFORE releasing the lock, so a second concurrent
     * esup_start observes threaded == true and cannot also spawn a thread. */
    engine->threaded = true;
    engine->stop = false;
    pthread_mutex_unlock(&engine->lock);

    if (pthread_create(&engine->thread, NULL, run_thread_main, engine) != 0) {
        ESUP_LOGE("driver thread create failed");
        pthread_mutex_lock(&engine->lock);
        engine->threaded = false;
        pthread_mutex_unlock(&engine->lock);
        return ESUP_SESSION_IO_ERROR;
    }
    return ESUP_OK;
}

esup_ret_t esup_stop(esup_engine_t* engine)
{
    if (engine == NULL) {
        ESUP_LOGE("engine is NULL");
        return ESUP_SESSION_ABORTED;
    }

    pthread_mutex_lock(&engine->lock);
    engine->stop = true;
    pthread_cond_broadcast(&engine->wake_cond);
    pthread_mutex_unlock(&engine->lock);

    if (engine->threaded) {
        pthread_join(engine->thread, NULL);
        engine->threaded = false;
    }

    pthread_mutex_lock(&engine->lock);
    engine->stop = false;
    pthread_mutex_unlock(&engine->lock);
    return ESUP_OK;
}

esup_ret_t esup_wait(esup_engine_t* engine, esup_session_t* session,
              uint32_t timeout_ms)
{
    if (engine == NULL || session == NULL) {
        ESUP_LOGE("bad argument (engine=%p session=%p)", (void*)engine,
                  (void*)session);
        return ESUP_SESSION_ABORTED;
    }

    uint32_t deadline = now_ms(engine) + timeout_ms;
    esup_ret_t res = ESUP_SESSION_PENDING;

    pthread_mutex_lock(&engine->lock);
    for (;;) {
        if (session->state == ESUP_SESSION_DONE) {
            res = session->res;
            break;
        }
        if (reached(engine, deadline))
            break;

        struct timespec ts;

        timespec_in(ESUP_WAIT_SLICE_MS, &ts);
        (void)pthread_cond_timedwait(&engine->done_cond, &engine->lock, &ts);
    }
    pthread_mutex_unlock(&engine->lock);
    return res;
}

/* ------------------------------------------------------------------------- */
/* Accessors                                                                 */
/* ------------------------------------------------------------------------- */

uint16_t esup_engine_module_id(const esup_engine_t* engine)
{
    return engine != NULL ? engine->module_id : 0u;
}

void esup_engine_set_module_id(esup_engine_t* engine, uint16_t module_id)
{
    if (engine == NULL)
        return;
    pthread_mutex_lock(&engine->lock);
    engine->module_id = module_id;
    pthread_mutex_unlock(&engine->lock);
}

bool esup_engine_is_running(const esup_engine_t* engine)
{
    return engine != NULL && engine->running;
}
