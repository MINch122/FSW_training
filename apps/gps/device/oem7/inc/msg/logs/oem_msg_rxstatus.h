#ifndef _OEM_MSG_RXSTATUS_H_
#define _OEM_MSG_RXSTATUS_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_RXSTATUS 93

/**
 * @struct oem_log_rxstatus_comp
 *
 * @brief One status code of a RXSTATUS log: the word itself and its three
 *        masks. The words repeat in a fixed order, receiver status first,
 *        then auxiliary 1 through 4.
 */
typedef struct {
    /**
     * @brief Status word.
     */
    oem_ulong           status;
    /**
     * @brief Priority mask, set by the STATUSCONFIG command.
     */
    oem_ulong           priority;
    /**
     * @brief Event set mask, set by the STATUSCONFIG command.
     */
    oem_ulong           eventSet;
    /**
     * @brief Event clear mask, set by the STATUSCONFIG command.
     */
    oem_ulong           eventClear;

} OEM_PACK oem_log_rxstatus_comp;

/**
 * @struct oem_log_rxstatus
 *
 * @brief Receiver status and error words. Asynch log. MID = 93.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    /**
     * @brief Receiver error word. Zero means no error; any set bit idles all
     *        channels and disables the RF hardware.
     */
    oem_ulong           error;
    /**
     * @brief # of status codes to follow, receiver status included.
     */
    oem_ulong           numStats;
    /**
     * @brief Status codes. See oem_log_rxstatus_comp.
     */
    oem_log_rxstatus_comp comp[];

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_rxstatus;

#endif
