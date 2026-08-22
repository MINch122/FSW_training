/**
 * @file esup_framer.h
 * @brief ESUP link layer: the frame codec plus a bidirectional framer bound to a
 *        wire.
 *
 * Two things live here. First, a pure, wire-free codec (CRC-32, encode, decode,
 * and the incremental reassembly primitives push/pop) that turns a noisy byte
 * stream into validated frames and back; it has no I/O and is unit-testable on
 * its own. Second, the framer object, which owns a wire and a tx staging buffer
 * and adds the two link-level operations the engine needs: esup_framer_send
 * (encode a frame and write it) and esup_framer_recv (read available bytes and
 * reassemble). Above this layer nothing touches the wire or a raw byte buffer.
 *
 * The reassembler handles the four hazards of reading ESUP off a raw stream:
 * leading line noise, late arrival, an incomplete long reply, and header-shaped
 * noise, via a HUNT -> LEN -> BODY -> CHECK scan with byte-wise resync.
 */
#ifndef ESUP_FRAMER_H
#define ESUP_FRAMER_H

#include <stddef.h>
#include <stdint.h>

#include "esup_proto.h"
#include "esup_wire.h"

/**
 * @brief Framer (link) return codes (bits 20-23 of esup_ret_t).
 */
enum {
    ESUP_FRAME_ERR_ARG      = 0x00100000, /**< Null pointer or impossible argument. */
    ESUP_FRAME_ERR_CAPACITY = 0x00200000, /**< Output buffer too small. */
    ESUP_FRAME_ERR_LENGTH   = 0x00300000, /**< Buffer length inconsistent with the header. */
    ESUP_FRAME_ERR_MAGIC    = 0x00400000, /**< Header magic not present. */
    ESUP_FRAME_ERR_CRC      = 0x00500000, /**< CRC-32 mismatch. */
    ESUP_FRAME_ERR_IO       = 0x00600000  /**< Wire transport error. */
};

/**
 * @brief The ESUP frame structure.
 *
 * Used both as the source for encoding and as the output of decoding. When
 * produced by the framer, @ref data points into the framer's internal buffer
 * and is valid only until the next framer call.
 */
typedef struct {
    uint16_t       module_id;  /** Module ID field. */
    uint16_t       cmd_status; /** Command Status field (ESUP_STATUS_*). */
    uint16_t       command;    /** Command field. */
    uint16_t       type;       /** Type field. */
    const void*    data;       /** Data field, or NULL when @ref data_len is 0. */
    uint16_t       data_len;   /** Length of the Data field in bytes. */
} esup_frame_t;

/* --- Pure codec (no I/O) ------------------------------------------------- */

/**
 * @brief Compute the ESUP CRC-32 over a range.
 * @return The 32-bit CRC.
 */
uint32_t esup_crc32(const void* data, size_t len);

/**
 * @brief Padded on-wire length of a frame carrying @p data_len data bytes.
 *
 * @param data_len Data field length.
 * @return align_up(ESUP_SIZE_OVERHEAD + data_len, ESUP_ALIGN), or 0 if
 *         @p data_len exceeds ESUP_DATA_MAX.
 */
size_t esup_frame_wire_len(uint16_t data_len);

/**
 * @brief Encode fields into a complete, padded, CRC-protected on-wire frame.
 *
 * @param frame    Fields to encode; data may be NULL when data_len is 0.
 * @param[out] out Destination buffer.
 * @param out_cap  Capacity of @p out in bytes.
 * @param[out] out_len  Total bytes written.
 * @return 0 on success; negative esup_frame_result_t on failure.
 */
esup_ret_t esup_frame_encode(const esup_frame_t* frame, uint8_t* out, size_t out_cap,
                      size_t* out_len);

/**
 * @brief Validate and decode one complete on-wire frame.
 *
 * @param buf Frame bytes (exactly one padded frame).
 * @param len Length of @p buf in bytes.
 * @param out Out: decoded fields, valid while @p buf is unchanged.
 * @return 0 on success; negative esup_frame_result_t on failure.
 */
esup_ret_t esup_frame_decode(const uint8_t* buf, size_t len, esup_frame_t* out);

/* --- The framer object (owns a wire, both directions) -------------------- */

/**
 * @brief Framer state: the reassembly buffer, a tx staging buffer, and the wire.
 *
 * Treat the fields as private; declared here only for static allocation. The
 * reassembly primitives (reset/push/pop) touch only the buffer and work without
 * a bound wire, which keeps the codec unit-testable.
 */
typedef struct {
    esup_wire_t wire;                /**< Bound transport (owned by value). */
    uint8_t     buf[ESUP_FRAME_MAX]; /**< Rx reassembly buffer. */
    size_t      len;                 /**< Valid bytes in @ref buf. */
    size_t      pending;             /**< Bytes of a just-returned frame to reclaim
                                          on the next call (deferred zero-copy). */
    uint8_t     tx[ESUP_FRAME_MAX];  /**< Tx staging for outbound frames. */
} esup_framer_t;

/**
 * @brief Bind a framer to an opened wire and reset it.
 *
 * @param framer Framer to initialise.
 * @param wire   An opened transport handle; copied into the framer.
 * @return 0 on success; ESUP_FRAME_ERR_ARG on a null argument.
 */
esup_ret_t esup_framer_init(esup_framer_t* framer, const esup_wire_t* wire);

/**
 * @brief Reset the reassembly state to empty (does not touch the wire).
 *
 * @param framer Framer to reset.
 */
void esup_framer_reset(esup_framer_t* framer);

/**
 * @brief Append freshly read bytes to the reassembler.
 *
 * On overflow the framer resyncs (drops from the front) so it always makes
 * forward progress. Wire-free; usable in tests without a bound wire.
 *
 * @param framer Framer to feed.
 * @param bytes  Newly read bytes.
 * @param n      Number of bytes.
 * @return 0 on success; negative esup_frame_result_t on argument error.
 */
esup_ret_t esup_framer_push(esup_framer_t* framer, const uint8_t* bytes, size_t n);

/**
 * @brief Extract the next complete, CRC-valid frame, if one is available.
 *
 * @param framer Framer to read from.
 * @param out    Out: decoded fields; data points into the framer buffer and is
 *               valid only until the next framer call.
 * @return 1 if a frame was produced; 0 otherwise (more bytes needed, or a null
 *         argument). This returns data, not an esup_ret_t code.
 */
int esup_framer_pop(esup_framer_t* framer, esup_frame_t* out);

/**
 * @brief Encode a frame and write it to the wire (the outbound link op).
 *
 * @param framer   Framer.
 * @param frame    Frame to send.
 * @param deadline Absolute monotonic send deadline.
 * @return 0 on success; ESUP_FRAME_ERR_IO on encode or transport failure.
 */
esup_ret_t esup_framer_send(esup_framer_t* framer, const esup_frame_t* frame,
                     uint32_t deadline);

/**
 * @brief Read available bytes from the wire into the reassembler (inbound link op).
 *
 * Blocks (only) until input arrives or @p deadline passes, then drains whatever
 * else is buffered without blocking. Call esup_framer_pop afterwards to extract
 * frames.
 *
 * @param framer   Framer.
 * @param deadline Absolute monotonic deadline to stop waiting for input.
 * @return 0 on success or timeout; ESUP_FRAME_ERR_IO on transport failure.
 */
esup_ret_t esup_framer_recv(esup_framer_t* framer, uint32_t deadline);

/**
 * @brief Read the wire's monotonic clock.
 *
 * @param framer Framer.
 * @return Milliseconds from an arbitrary fixed origin.
 */
uint32_t esup_framer_now(const esup_framer_t* framer);

/**
 * @brief Change the wire's line rate.
 *
 * @param framer Framer.
 * @param baud   New line rate in bit/s.
 * @return 0 on success; ESUP_FRAME_ERR_IO on failure or if unsupported.
 */
esup_ret_t esup_framer_set_baud(esup_framer_t* framer, uint32_t baud);

#endif /* ESUP_FRAMER_H */
