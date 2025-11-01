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
 * @brief Number of the physical ports. Normally 1 to use the COM1 port only.
 *        Sets the limiting index for OEM_InitPhysicalPort().
 */
#define OEM_PHYSICAL_PORTS                  1

/**
 * Three kinds of buffers, each with corresponding size defs are present.
 *
 * 1) Serial Read Buffer (malloc): OEM_UTILS_READBUF_SIZE
 *      This buffer defined in oem_utils.c serves as an intermediate storage
 *      for incoming serial bytes. The buffer always retrieve a large chunk of
 *      data in order to avoid too many read syscalls during the sync word
 *      search.
 *      No strict size limit, but one or more page sizes is recommended for
 *      an improved performance.
 *
 * 2) Read Task Message Buffer (static): OEM_TASK_MSG_BUF_SIZE
 *      Temporarily stores a message received before passing it to a log
 *      handler. Should be large enough to hold any complete OEM message.
 *
 * 3) Handler Recent Msg Buffer (malloc): OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE
 *      This buffer is attached to each handler and stores the latest log
 *      received. The size macro applies only to variable-lengthed logs and
 *      defines the maximum length a log is truncated by. Header and CRC
 *      exclusive. Should be equal or less than OEM_TASK_MSG_BUF_SIZE.
 */
#define OEM_UTILS_READBUF_SIZE              4096
#define OEM_TASK_MSG_BUF_SIZE               1024
#define OEM_LOG_HANDLER_RECENT_MSG_MAX_SIZE 1024

/**
 * @brief UART read timeout (ms).
 */
#define OEM_SERIAL_READ_TIMEOUT             1000

/**
 * @brief Maximum number of the handler objects.
 */
#define OEM_LOG_HANDLER_MAX                 20

/**
 * @brief Handler name field length. //TODO: grow this.
 */
#define OEM_LOG_HANDLER_NAME_LEN            8

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
