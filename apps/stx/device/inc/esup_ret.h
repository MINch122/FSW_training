/**
 * @file esup_ret.h
 * @brief Universal ESUP driver return code and its layered layout.
 *
 * Every result-returning function in the driver returns esup_ret_t. Success is
 * ESUP_OK (0). A failure is a partitioned 32-bit value: each layer owns a field,
 * and a lower layer ORs its own cause into the value as it propagates up, so one
 * code carries the whole causal stack without losing information.
 *
 *   byte 3        byte 2              byte 1          byte 0
 *   [  engine  ]  [ frame ][ wire ] [ device-local ][ exec status ]
 *   bits 31-24    23-20     19-16    bits 15-8       bits 7-0
 *   └──────────── ESUP driver ─────┘└────────── device ────────┘
 *
 * The ESUP driver owns the two most-significant bytes (wire / framer / engine);
 * a custom device driver (e.g. stx) owns byte 1. The module's raw one-byte
 * command-execution status occupies byte 0. The device never writes the driver
 * region, so codes from different fields never collide.
 *
 * The engine extracts the execution status because it is part of the generic
 * Get_Results structure: zero means execution success and nonzero produces
 * ESUP_SESSION_EXEC_ERROR. Its command-specific meaning remains the device
 * driver's responsibility and is read with ESUP_RET_EXEC_STATUS().
 *
 * Test for failure with `ret != ESUP_OK`. Never test the sign: the value is a bit
 * field, not a signed magnitude, so `< 0` is meaningless here.
 */
#ifndef ESUP_RET_H
#define ESUP_RET_H

#include <stdint.h>

/** @brief Universal driver return code (see the layout above). */
typedef int32_t esup_ret_t;

/** @brief Success, for every layer. */
#define ESUP_OK 0

/* Field masks. */
#define ESUP_RET_ENGINE_MASK       0xFF000000u /**< Engine / transport byte. */
#define ESUP_RET_FRAME_MASK        0x00F00000u /**< Framer (link) nibble. */
#define ESUP_RET_WIRE_MASK         0x000F0000u /**< Wire nibble. */
#define ESUP_RET_DEVICE_MASK       0x0000FFFFu /**< Complete device-owned region. */
#define ESUP_RET_DEVICE_LOCAL_MASK 0x0000FF00u /**< Device-driver local byte. */
#define ESUP_RET_EXEC_STATUS_MASK  0x000000FFu /**< Module execution-status byte. */

/** @brief Isolate the engine/transport field of a return code. */
#define ESUP_RET_ENGINE(r) ((esup_ret_t)((uint32_t)(r) & ESUP_RET_ENGINE_MASK))
/** @brief Isolate the framer field of a return code. */
#define ESUP_RET_FRAME(r)  ((esup_ret_t)((uint32_t)(r) & ESUP_RET_FRAME_MASK))
/** @brief Isolate the wire field of a return code. */
#define ESUP_RET_WIRE(r)   ((esup_ret_t)((uint32_t)(r) & ESUP_RET_WIRE_MASK))
/** @brief Isolate the device / custom-driver field of a return code. */
#define ESUP_RET_DEVICE(r) ((esup_ret_t)((uint32_t)(r) & ESUP_RET_DEVICE_MASK))
/** @brief Isolate a device driver's local-error field. */
#define ESUP_RET_DEVICE_LOCAL(r) \
    ((esup_ret_t)((uint32_t)(r) & ESUP_RET_DEVICE_LOCAL_MASK))
/** @brief Extract the module's raw command-execution status. */
#define ESUP_RET_EXEC_STATUS(r) \
    ((uint8_t)((uint32_t)(r) & ESUP_RET_EXEC_STATUS_MASK))

#endif /* ESUP_RET_H */
