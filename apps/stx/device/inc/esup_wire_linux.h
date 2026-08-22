/**
 * @file esup_wire_linux.h
 * @brief POSIX/Linux serial backend for the ESUP wire layer.
 *
 * Binds esup_wire_t to a character device (RS-485 or UART) using termios2 for
 * arbitrary baud rates, poll() for deadline-bounded I/O, and CLOCK_MONOTONIC
 * for timing. The context is caller-owned storage, so no dynamic allocation is
 * performed and the backend is usable on constrained embedded Linux.
 */
#ifndef ESUP_WIRE_LINUX_H
#define ESUP_WIRE_LINUX_H

#include <stdbool.h>
#include <stdint.h>

#include "esup_wire.h"

/** Enable kernel RS-485 auto-direction (RTS toggled per frame) via TIOCSRS485. */
#define ESUP_WIRE_LINUX_RS485 0x1u

/**
 * @brief Backend context.
 *
 * Fields are private; declared here only so the caller can allocate one
 * statically. Do not touch after esup_wire_linux_open.
 */
typedef struct {
    int      fd;        /**< Open device descriptor, or -1. */
    uint32_t baud;      /**< Last rate applied, for diagnostics. */
    bool     rs485;     /**< Hardware RS-485 direction is enabled. */
    bool     null_mode; /**< Offline /dev/null mode: serial config and drain skipped. */
} esup_wire_linux_ctx_t;

/**
 * @brief Configure a serial device and bind it to a wire handle.
 *
 * Configures raw 8N1, no flow control, VMIN=0/VTIME=0 (the backend polls against
 * explicit deadlines). Arbitrary baud rates are set through termios2/BOTHER, so
 * ESUP's non-standard rates (e.g. 250000) work without Bxxx macros.
 *
 * Offline mode: if @p path is "/dev/null" the device is opened but all serial
 * configuration (termios2, RS-485, baud) is skipped, sends are discarded without
 * draining, and reads block out their deadline and report a timeout. This lets
 * the engine and higher layers run without hardware — every transaction simply
 * times out to UNREACHABLE. @p baud and @p flags are ignored in this mode.
 *
 * @param ctx   Caller-provided storage, kept alive for the life of the handle.
 * @param out   Handle to initialise; on success out->ops and out->ctx are set.
 * @param path  Device node, e.g. "/dev/ttyS0" or "/dev/ttyUSB0".
 * @param baud  Line rate in bit/s (ESUP default 3000000; table 250000, 500000,
 *              1000000, 2000000, 3000000).
 * @param flags Bitwise OR of ESUP_WIRE_LINUX_* (0 for a plain UART).
 * @return ESUP_OK on success; a wire field code on failure, with errno set by
 *         the failing system call. On failure the device is closed.
 */
esup_ret_t esup_wire_linux_open(esup_wire_linux_ctx_t* ctx, esup_wire_t* out,
                                const char* path, uint32_t baud, unsigned flags);

#endif /* ESUP_WIRE_LINUX_H */
