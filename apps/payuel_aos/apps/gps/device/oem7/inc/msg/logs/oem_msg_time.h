#ifndef _OEM_MSG_TIME_H_
#define _OEM_MSG_TIME_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_TIME 101

/**
 * @struct oem_log_time
 *
 * @brief Several time related pieces. Synch log. MID = 101.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief Clock model status (not including current measurement data), 
     *        see Table 95: Clock Model Status on page 441.
     */
    oem_enum            clockStatus;

    /**
     * @brief Receiver clock offset from the GPS system time in seconds.
     *        GPS system time = GPS reference time (in header) - offset.
     */
    oem_double          Offset;

    /**
     * @brief Receiver clock offset standard deviation (s).
     */
    oem_double          Offsetstd;

    /**
     * @brief GPS system time offset from UTC time in seconds
     *        UTC time = GPS system time + UTC offset
     *                 = GPS reference time - offset + UTC offset.
     */
    oem_double          UtcOffset;

    /**
     * @brief UTC year.
     */
    oem_ulong           UtcYear;

    /**
     * @brief UTC month from 0 to 12. 0 if UTC time is unknown.
     */
    oem_uchar           UtcMonth;

    /**
     * @brief UTC day from 0 to 31. 0 if UTC time is unknown.
     */
    oem_uchar           UtcDay;

    /**
     * @brief UTC hour from 0 to 23.
     */
    oem_uchar           UtcHour;

    /**
     * @brief UTC minute from 0 to 59.
     */
    oem_uchar           UtcMin;

    /**
     * @brief UTC milliseconds from 0 to 60999. 
     *        Maximum 60999 when leap second is applied.
     */
    oem_ulong           UtcMs;

    /**
     * @brief UTC status.  0: Invalid, 1: Valid, 
     *        2: Warning (indicates leap second is applied as default)
     * 
     */
    oem_enum            UtcStatus;

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_time;

#endif