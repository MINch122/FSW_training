#ifndef _OEM_MSG_HWMONITOR_H_
#define _OEM_MSG_HWMONITOR_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_HWMONITOR 963

typedef enum {
    HWMONITOR_VALUE_ACCEPTABLE      = 0x0000,
    HWMONITOR_VALUE_WARNING_LOWER   = 0x0001,
    HWMONITOR_VALUE_ERROR_LOWER     = 0x0002,
    HWMONITOR_VALUE_WARNING_UPPER   = 0x0003,
    HWMONITOR_VALUE_ERROR_UPPER     = 0x0004,
    HWMONITOR_READING_PRI_TEMP      = 0x0100,
    HWMONITOR_READING_ANT_CURR      = 0x0200,
    HWMONITOR_READING_ANT_VOLT      = 0x0700,
    HWMONITOR_READING_DIGIT_CORE_VOLT = 0x0800,
    HWMONITOR_READING_SUPPLY_VOLT   = 0x0F00,
    HWMONITOR_READING_SEC_TEMP      = 0x1600,
    HWMONITOR_READING_PERIPH_CORE_VOLT = 0x1700,
    HWMONITOR_READING_SEC_ANT_CURR  = 0x1800, /* OEM7720 */
    HWMONITOR_READING_SEC_ANT_VOLT  = 0x1900, /* OEM7720 */
} hwmonitor_status_mask_t;

/**
 * @brief HWMONITOR log component. See oem_log_hwmonitor.
 */
typedef struct {
    /**
     * @brief Temperature (C), antenna current (A) or voltage (V).
     */
    oem_float           reading;

    /**
     * @brief Monitor status.
     *        0x00 = Value falls within acceptable bounds.
     *        0x01 = Value is under the lower warning limit.
     *        0x02 = Value is under the lower error limit.
     *        0x03 = Value is over the upper warning limit.
     *        0x04 = Value is over the upper error limit.
     */
    oem_ulong           status;
} OEM_PACK oem_log_hwmonitor_comp;

/**
 * @struct oem_log_hwmonitor
 *
 * @brief Monitor hardware levels. Polled log. MID = 963.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    /**
     * @brief Number of measurements to follow.
     */
    oem_ulong numMeasurements;

    oem_log_hwmonitor_comp comp[];

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_hwmonitor;

#endif