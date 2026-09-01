/**
 * @file oem_config.h
 * @brief OEM7 driver user configurations.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_CONFIG_H_
#define _OEM_CONFIG_H_

/**
 * @brief Enables debugging log messages to the standard output.
 */
#define OEM_DEBUG                           true

/**
 * @brief Number of the physical interfaces. Normally 1 to use the COM1 port
 *        only. Sets the limiting index for interfaces in the I/O layer.
 */
#define OEM_IO_INTERFACES                      1

/**
 * @brief Size, in bytes, of the receive state machine's message buffer.
 *
 *        Each I/O interface owns one such buffer. As bytes arrive, the task
 *        layer assembles a single complete message into it (header + body +
 *        CRC). It must be large enough for the largest OEM7 message the
 *        driver is expected to receive; a message whose total length exceeds
 *        this size is rejected with OEM_ERR_TOO_LARGE.
 */
#define OEM_TASK_STATE_MACHINE_BUF_SIZE     2048

/**
 * @brief Size, in bytes, of a log handler's most-recent-message buffer for
 *        variable-length messages.
 *
 *        Every handler stores a copy of the last message it received. A 
 *        handler for variable-length messages (OEM_LOG_HANDLER_MLEN_VARIABLE)
 *        allocates a buffer of this size; a message longer than this is
 *        truncated when stored.
 *        A fixed-length message handler allocates a buffer of exactly that 
 *        length and this macro does not apply to it. 
 */
#define OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE 2048

/**
 * @brief Default size, in bytes, of an I/O interface's read buffer.
 *
 *        The read buffer holds raw bytes fetched from the interface before
 *        the task layer consumes them, batching reads to reduce unnecessarily
 *        frequent read() syscalls. The read buffer size is chosen by the
 *        read_buf_size argument of oem_io_init_interface(); passing 0 there
 *        selects this default. No strict size requirement, but one or more
 *        pages is recommended.
 */
#define OEM_IO_DEFAULT_READBUF_SIZE         8192

/**
 * @brief Maximum number of the handler objects.
 */
#define OEM_LOG_HANDLER_MAX                 20

/**
 * @brief Handler name field length. Null terminator inclusive.
 */
#define OEM_LOG_HANDLER_NAME_LEN            16

/**
 * @brief Mission-specific Log & Command Message Type.
 */
#define OEM_MISSION_MSGTYPE                 ((OEM_MSGTYPE_BINARY) | \
                                             (OEM_MSGTYPE_ORIGINAL))

/**
 * @brief Three sync bytes at the start of the header.
 */
#define OEM_SYNC_BYTE1                      0xAA
#define OEM_SYNC_BYTE2                      0x44
#define OEM_SYNC_BYTE3                      0x12

/**
 * @brief If true, use a lookup table for CRC32 calculation, else bit-by-bit
 *        as per the manual implementation. The lookup consumes static memory
 *        of 1024 bytes.
 */
#define OEM_CRC32_USE_LOOKUP                true

#endif
