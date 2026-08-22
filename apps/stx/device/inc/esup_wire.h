/**
 * @file esup_wire.h
 * @brief Transport (wire) abstraction for the EnduroSat User Protocol.
 *
 * The only contract between the ESUP engine and the physical link. The engine
 * owns framing, CRC, padding, retries and timing policy; the wire layer moves
 * opaque bytes and reports monotonic time. Nothing ESUP-specific lives below
 * this boundary, which is what makes the serial backend replaceable.
 *
 * A backend is a pair (ops, ctx): a vtable and an opaque context passed back to
 * every call. Result-returning ops return esup_ret_t (ESUP_OK on success, a wire
 * field code otherwise). Deadlines are absolute monotonic timestamps in
 * milliseconds; comparisons are wrap-safe, so treat
 * (int32_t)(deadline_ms - now_ms) <= 0 as expired.
 */
#ifndef ESUP_WIRE_H
#define ESUP_WIRE_H

#include <stddef.h>
#include <stdint.h>

#include "esup_ret.h"

/**
 * @brief Wire-layer return codes (bits 16-19 of esup_ret_t).
 */
enum {
    ESUP_WIRE_ERR_TIMEOUT     = 0x00010000, /**< Deadline reached before completion. */
    ESUP_WIRE_ERR_IO          = 0x00020000, /**< Unrecoverable transport error. */
    ESUP_WIRE_ERR_UNSUPPORTED = 0x00030000  /**< Optional capability not provided. */
};

/**
 * @brief Backend vtable; each function receives the backend's opaque context.
 */
typedef struct {
    /**
     * @brief Transmit exactly @p len bytes.
     *
     * Blocks, bounded by @p deadline_ms, until all bytes have been clocked out
     * and the local transmitter is idle, or the deadline passes, or the link
     * fails. On a half-duplex link the whole frame must have left the wire and
     * the local driver must be released before returning success, so a following
     * read cannot capture the local echo. A @p len of 0 is a no-op.
     *
     * @param ctx         Backend context.
     * @param buf         Bytes to send.
     * @param len         Number of bytes to send.
     * @param deadline_ms Absolute monotonic deadline in milliseconds.
     * @return ESUP_OK; ESUP_WIRE_ERR_TIMEOUT if the deadline was reached (an
     *         unknown prefix may have been sent); ESUP_WIRE_ERR_IO on error.
     */
    esup_ret_t (*write)(void* ctx, const uint8_t* buf, size_t len, uint32_t deadline_ms);

    /**
     * @brief Receive up to @p cap bytes without requiring a specific count.
     *
     * Blocks, bounded by @p deadline_ms, until at least one byte is available,
     * then returns whatever is buffered up to @p cap. Never interprets framing.
     * @p got is always written (0 on timeout).
     *
     * @param ctx         Backend context.
     * @param buf         Destination buffer.
     * @param cap         Capacity of @p buf in bytes.
     * @param got         Out: number of bytes copied into @p buf.
     * @param deadline_ms Absolute monotonic deadline in milliseconds.
     * @return ESUP_OK with *got > 0; ESUP_WIRE_ERR_TIMEOUT with *got == 0 on
     *         deadline; ESUP_WIRE_ERR_IO on error.
     */
    esup_ret_t (*read)(void* ctx, uint8_t* buf, size_t cap, size_t* got, uint32_t deadline_ms);

    /**
     * @brief Monotonic milliseconds.
     *
     * Must be monotonic and immune to wall-clock or NTP adjustments. Resolution
     * should be <= 1 ms, but engine correctness must not depend on sub-ms
     * accuracy.
     *
     * @param ctx Backend context.
     * @return Milliseconds from an arbitrary fixed origin.
     */
    uint32_t (*now_ms)(void* ctx);

    /**
     * @brief Change the line rate (for the Change_Baud_Rate command).
     *
     * Invoked only between transactions with no bytes in flight. The backend
     * must drain residual output and flush input before switching.
     *
     * @param ctx  Backend context.
     * @param baud New line rate in bit/s.
     * @return ESUP_OK; ESUP_WIRE_ERR_UNSUPPORTED for a fixed-rate backend;
     *         ESUP_WIRE_ERR_IO on error.
     */
    esup_ret_t (*set_baud)(void* ctx, uint32_t baud);

    /**
     * @brief Release all resources held by @p ctx. Idempotent.
     *
     * @param ctx Backend context.
     */
    void (*close)(void* ctx);
} esup_wire_ops_t;

/**
 * @brief A transport handle: an implementation bound to its context.
 */
typedef struct {
    const esup_wire_ops_t* ops; /**< Backend vtable. */
    void*                ctx; /**< Backend opaque context. */
} esup_wire_t;

/**
 * @brief Transmit a full frame. See esup_wire_ops_t::write.
 *
 * @param wire        Transport handle.
 * @param buf         Bytes to send.
 * @param len         Number of bytes to send.
 * @param deadline_ms Absolute monotonic deadline in milliseconds.
 * @return ESUP_OK, or a wire field code on failure.
 */
static inline esup_ret_t esup_wire_write(const esup_wire_t* wire, const uint8_t* buf,
                                         size_t len, uint32_t deadline_ms)
{
    return wire->ops->write(wire->ctx, buf, len, deadline_ms);
}

/**
 * @brief Receive available bytes. See esup_wire_ops_t::read.
 *
 * @param wire        Transport handle.
 * @param buf         Destination buffer.
 * @param cap         Capacity of @p buf in bytes.
 * @param got         Out: number of bytes copied.
 * @param deadline_ms Absolute monotonic deadline in milliseconds.
 * @return ESUP_OK, or a wire field code on failure.
 */
static inline esup_ret_t esup_wire_read(const esup_wire_t* wire, uint8_t* buf, size_t cap,
                                        size_t* got, uint32_t deadline_ms)
{
    return wire->ops->read(wire->ctx, buf, cap, got, deadline_ms);
}

/**
 * @brief Read the monotonic clock. See esup_wire_ops_t::now_ms.
 *
 * @param wire Transport handle.
 * @return Milliseconds from an arbitrary fixed origin.
 */
static inline uint32_t esup_wire_now_ms(const esup_wire_t* wire)
{
    return wire->ops->now_ms(wire->ctx);
}

/**
 * @brief Change the line rate. See esup_wire_ops_t::set_baud.
 *
 * @param wire Transport handle.
 * @param baud New line rate in bit/s.
 * @return ESUP_OK, or a wire field code on failure.
 */
static inline esup_ret_t esup_wire_set_baud(const esup_wire_t* wire, uint32_t baud)
{
    if (wire->ops->set_baud == NULL)
        return ESUP_WIRE_ERR_UNSUPPORTED;
    return wire->ops->set_baud(wire->ctx, baud);
}

/**
 * @brief Release the backend. See esup_wire_ops_t::close.
 *
 * @param wire Transport handle.
 */
static inline void esup_wire_close(const esup_wire_t* wire)
{
    wire->ops->close(wire->ctx);
}

#endif /* ESUP_WIRE_H */
