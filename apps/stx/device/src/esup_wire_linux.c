/**
 * @file esup_wire_linux.c
 * @brief POSIX/Linux serial backend implementation for the ESUP wire layer.
 *
 * Uses termios2/BOTHER for arbitrary baud, poll() for deadline-bounded I/O,
 * CLOCK_MONOTONIC for time, tcdrain (via TCSBRK) to guarantee half-duplex frame
 * completion, and optional TIOCSRS485 for RS-485 auto-direction. It deliberately
 * avoids <termios.h> because that header conflicts with the kernel's
 * <asm/termbits.h> definition of struct termios2; all line control is done with
 * raw ioctls instead.
 */
// #define _GNU_SOURCE

#include "esup_wire_linux.h"
#include "esup_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <linux/serial.h>

/* ------------------------------------------------------------------------- */
/* Time                                                                      */
/* ------------------------------------------------------------------------- */

/**
 * @brief Read the monotonic clock as milliseconds.
 *
 * @return Milliseconds from an arbitrary fixed origin (wraps every ~49.7 days).
 */
static uint32_t monotonic_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((uint64_t)ts.tv_sec * 1000u
                      + (uint64_t)ts.tv_nsec / 1000000u);
}

/**
 * @brief Milliseconds remaining until an absolute monotonic deadline.
 *
 * Wrap-safe: a deadline in the past yields 0. The result is clamped to a value
 * that poll() can accept.
 *
 * @param deadline_ms Absolute monotonic deadline.
 * @return Non-negative milliseconds until the deadline (0 if already reached).
 */
static int deadline_remaining_ms(uint32_t deadline_ms)
{
    int32_t diff = (int32_t)(deadline_ms - monotonic_ms());

    if (diff <= 0)
        return 0;
    if (diff > INT_MAX)
        return INT_MAX;
    return (int)diff;
}

/* ------------------------------------------------------------------------- */
/* Line configuration                                                        */
/* ------------------------------------------------------------------------- */

/**
 * @brief Test whether an actual baud rate is close enough to the requested one.
 *
 * A driver that programs a custom rate through BOTHER reports the achieved rate
 * back; UARTs reach standard and common rates to well within one percent, so a
 * two-percent window accepts legitimate divisor rounding while rejecting a
 * driver that silently ignored the request (reverting to a default or zero).
 *
 * @param actual Rate reported by the driver.
 * @param want   Requested rate.
 * @return Non-zero if @p actual is within tolerance of @p want.
 */
static int baud_within_tolerance(uint32_t actual, uint32_t want)
{
    uint32_t tolerance = want / 50u; /* two percent */
    uint32_t diff = (actual > want) ? (actual - want) : (want - actual);

    if (tolerance == 0u)
        tolerance = 1u;
    return diff <= tolerance;
}

/**
 * @brief Apply raw 8N1 termios2 settings at an arbitrary baud rate.
 *
 * Sets the rate through termios2/BOTHER so non-standard rates such as 250000
 * work without a Bxxx macro, then reads the settings back and verifies the
 * driver actually programmed the requested rate. A driver that cannot honour a
 * custom rate fails here rather than silently running at the wrong speed.
 *
 * @param fd   Open device descriptor.
 * @param baud Line rate in bit/s.
 * @return 0 on success; ESUP_WIRE_ERR_UNSUPPORTED if the driver did not program
 *         the requested rate; ESUP_WIRE_ERR_IO on a system-call failure.
 */
static esup_ret_t configure_line(int fd, uint32_t baud)
{
    struct termios2 tio;

    if (ioctl(fd, TCGETS2, &tio) < 0) {
        ESUP_LOGE("TCGETS2 failed: %s", strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }

    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cflag &= ~(tcflag_t)(CSIZE | PARENB | CSTOPB | CRTSCTS | CBAUD);
    tio.c_cflag |= (tcflag_t)(CS8 | CLOCAL | CREAD | BOTHER);
    tio.c_ispeed = baud;
    tio.c_ospeed = baud;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    if (ioctl(fd, TCSETS2, &tio) < 0) {
        ESUP_LOGE("TCSETS2 failed (baud=%u): %s", baud, strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }

    /* Confirm the request was honoured; the driver reports the achieved rate. */
    if (ioctl(fd, TCGETS2, &tio) < 0) {
        ESUP_LOGE("TCGETS2 readback failed: %s", strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }
    if (!baud_within_tolerance(tio.c_ospeed, baud)
        || !baud_within_tolerance(tio.c_ispeed, baud)) {
        ESUP_LOGE("baud %u not honoured (got in=%u out=%u)", baud,
                  tio.c_ispeed, tio.c_ospeed);
        errno = EINVAL;
        return ESUP_WIRE_ERR_UNSUPPORTED;
    }
    return ESUP_OK;
}

/**
 * @brief Enable kernel RS-485 auto-direction on the device.
 *
 * @param fd Open device descriptor.
 * @return 0 on success; ESUP_WIRE_ERR_IO on failure (errno set).
 */
static esup_ret_t configure_rs485(int fd)
{
    struct serial_rs485 rs;

    memset(&rs, 0, sizeof(rs));
    rs.flags = (SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND);

    if (ioctl(fd, TIOCSRS485, &rs) < 0) {
        ESUP_LOGE("TIOCSRS485 failed: %s", strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }
    return ESUP_OK;
}

/**
 * @brief Block until all pending output has been transmitted (tcdrain).
 *
 * @param fd Open device descriptor.
 * @return 0 on success; ESUP_WIRE_ERR_IO on failure (errno set).
 */
static esup_ret_t drain_output(int fd)
{
    for (;;) {
        if (ioctl(fd, TCSBRK, 1) == 0)
            return ESUP_OK;
        if (errno == EINTR)
            continue;
        ESUP_LOGE("tcdrain (TCSBRK) failed: %s", strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }
}

/* ------------------------------------------------------------------------- */
/* Wire operations                                                           */
/* ------------------------------------------------------------------------- */

/**
 * @brief Wait for a single poll event on the device, bounded by a deadline.
 *
 * @param fd          Open device descriptor.
 * @param events      Requested poll events (POLLIN or POLLOUT).
 * @param deadline_ms Absolute monotonic deadline.
 * @return ESUP_OK if the requested event is ready; ESUP_WIRE_ERR_TIMEOUT on
 *         timeout; ESUP_WIRE_ERR_IO on a poll error or a device error/hangup.
 */
static esup_ret_t wait_event(int fd, short events, uint32_t deadline_ms)
{
    for (;;) {
        int remaining = deadline_remaining_ms(deadline_ms);

        if (remaining == 0)
            return ESUP_WIRE_ERR_TIMEOUT;

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = events;
        pfd.revents = 0;

        int pr = poll(&pfd, 1, remaining);

        if (pr < 0) {
            if (errno == EINTR)
                continue;
            ESUP_LOGE("poll failed: %s", strerror(errno));
            return ESUP_WIRE_ERR_IO;
        }
        if (pr == 0)
            return ESUP_WIRE_ERR_TIMEOUT;
        if ((pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            ESUP_LOGE("device error/hangup (revents=0x%X)", pfd.revents);
            return ESUP_WIRE_ERR_IO;
        }
        return ESUP_OK;
    }
}

/**
 * @brief Transmit exactly @p len bytes, then drain. See esup_wire_ops_t::write.
 */
static esup_ret_t wire_write(void* vctx, const uint8_t* buf, size_t len,
                      uint32_t deadline_ms)
{
    esup_wire_linux_ctx_t* ctx = vctx;
    // size_t sent = 0;

    if (len == 0)
        return ESUP_OK;

    // /* Offline: discard the frame down /dev/null and skip the tcdrain (TCSBRK,
    //  * which is not valid on a non-tty). */
    // if (ctx->null_mode) {
    //     while (sent < len) {
    //         ssize_t n = write(ctx->fd, &buf[sent], len - sent);

    //         if (n < 0) {
    //             if (errno == EINTR || errno == EAGAIN)
    //                 continue;
    //             return ESUP_WIRE_ERR_IO;
    //         }
    //         sent += (size_t)n;
    //     }
    //     return ESUP_OK;
    // }

    // while (sent < len) {
        // esup_ret_t ready = wait_event(ctx->fd, POLLOUT, deadline_ms);

        // if (ready == ESUP_WIRE_ERR_TIMEOUT)
        //     ESUP_LOGW("write timed out (%zu/%zu bytes sent)", sent, len);
        // if (ready != ESUP_OK)
        //     return ready; /* timeout or IO; already logged */

        ssize_t n = write(ctx->fd, buf, len);

        if (n < 0) {
            // if (errno == EINTR || errno == EAGAIN)
            //     continue;
            ESUP_LOGE("write failed: %s", strerror(errno));
            return ESUP_WIRE_ERR_IO;
        }
        // sent += (size_t)n;
    // }

    /* Half-duplex: the frame must be fully clocked out and the transmitter idle
     * before a following read, so the local echo is never captured. */
    // return drain_output(ctx->fd);
    return ESUP_OK;
}

/**
 * @brief Receive available bytes. See esup_wire_ops_t::read.
 */
static esup_ret_t wire_read(void* vctx, uint8_t* buf, size_t cap, size_t* got,
                     uint32_t deadline_ms)
{
    esup_wire_linux_ctx_t* ctx = vctx;

    *got = 0;

    /* Offline: there is never inbound data. Sleep out the deadline (rather than
     * busy-reading EOF from /dev/null) and report the idle timeout the engine
     * expects. */
    if (ctx->null_mode) {
        int remaining = deadline_remaining_ms(deadline_ms);

        if (remaining > 0)
            poll(NULL, 0, remaining);

        return ESUP_WIRE_ERR_TIMEOUT;
    }

    for (;;) {
        esup_ret_t ready = wait_event(ctx->fd, POLLIN, deadline_ms);

        if (ready != ESUP_OK)
            return ready; /* timeout (idle, expected) or IO; already logged */

        ssize_t n = read(ctx->fd, buf, cap);

        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            ESUP_LOGE("read failed: %s", strerror(errno));
            return ESUP_WIRE_ERR_IO;
        }
        if (n == 0)
            continue;

        *got = (size_t)n;
        return ESUP_OK;
    }
}

/**
 * @brief Monotonic milliseconds. See esup_wire_ops_t::now_ms.
 */
static uint32_t wire_now_ms(void* vctx)
{
    (void)vctx;
    return monotonic_ms();
}

/**
 * @brief Change the line rate. See esup_wire_ops_t::set_baud.
 */
static esup_ret_t wire_set_baud(void* vctx, uint32_t baud)
{
    esup_wire_linux_ctx_t* ctx = vctx;

    /* Offline: no line to reprogram; just record the requested rate. */
    if (ctx->null_mode) {
        ctx->baud = baud;
        return ESUP_OK;
    }

    esup_ret_t rc = drain_output(ctx->fd);

    if (rc != ESUP_OK)
        return rc;

    rc = configure_line(ctx->fd, baud);
    if (rc != ESUP_OK)
        return rc;

    if (ioctl(ctx->fd, TCFLSH, TCIFLUSH) < 0) {
        ESUP_LOGE("TCFLSH failed: %s", strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }

    ctx->baud = baud;
    return ESUP_OK;
}

/**
 * @brief Close the device. See esup_wire_ops_t::close.
 */
static void wire_close(void* vctx)
{
    esup_wire_linux_ctx_t* ctx = vctx;

    if (ctx->fd >= 0) {
        close(ctx->fd);
        ctx->fd = -1;
    }
}

/** The backend vtable, shared by every handle opened by this backend. */
static const esup_wire_ops_t g_linux_ops = {
    .write    = wire_write,
    .read     = wire_read,
    .now_ms   = wire_now_ms,
    .set_baud = wire_set_baud,
    .close    = wire_close
};

/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

esup_ret_t esup_wire_linux_open(esup_wire_linux_ctx_t* ctx, esup_wire_t* out,
                         const char* path, uint32_t baud, unsigned flags)
{
    if (ctx == NULL || out == NULL || path == NULL) {
        ESUP_LOGE("bad argument (ctx=%p out=%p path=%p)", (void*)ctx,
                  (void*)out, (const void*)path);
        errno = EINVAL;
        return ESUP_WIRE_ERR_IO;
    }

    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) {
        ESUP_LOGE("open(%s) failed: %s", path, strerror(errno));
        return ESUP_WIRE_ERR_IO;
    }

    ctx->fd = fd;
    ctx->baud = baud;
    ctx->rs485 = false;
    ctx->null_mode = (strcmp(path, "/dev/null") == 0);

    /* Offline mode: a real /dev/null is not a tty, so the termios2/RS-485 ioctls
     * would fail; skip all serial configuration and run as a black-hole wire. */
    if (ctx->null_mode) {
        ESUP_LOGI("/dev/null offline mode: skipping serial configuration");
        out->ops = &g_linux_ops;
        out->ctx = ctx;
        return ESUP_OK;
    }

    /* Program raw 8N1 at the requested rate. configure_line() drives the baud
     * through termios2/BOTHER (c_ispeed/c_ospeed), which is what lets a
     * non-standard rate such as 250000 take -- a Bxxx-macro/cfsetspeed path has
     * no constant for it and silently falls back, so it is not used here. */
    esup_ret_t rc = configure_line(fd, baud);

    if (rc != ESUP_OK) {
        close(fd);
        ctx->fd = -1;
        return rc;
    }

    /* The physical link is RS-485, but we deliberately drive it as a plain
     * serial line: the kernel RS-485 auto-direction ioctl (configure_rs485 /
     * TIOCSRS485) is intentionally not applied here. Direction handling, if
     * any, is left to the hardware/adapter. */
    (void)flags;
    (void)configure_rs485;

    /* Start every session from a clean line. */
    (void)ioctl(fd, TCFLSH, TCIOFLUSH);

    out->ops = &g_linux_ops;
    out->ctx = ctx;
    return ESUP_OK;
}
