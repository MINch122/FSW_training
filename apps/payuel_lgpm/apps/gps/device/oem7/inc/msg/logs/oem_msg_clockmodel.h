#ifndef _OEM_MSG_CLOCKMODEL_H_
#define _OEM_MSG_CLOCKMODEL_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_CLOCKMODEL 16

/**
 * @struct oem_log_clockmodel
 *
 * @brief Current clock model status. Synch log. MID = 16.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief Clock model status. See Table 86 (Clock Model Status).
     */
    oem_enum            clockStatus;

    /**
     * @brief Number of rejected range bias measurements.
     */
    oem_ulong           reject;

    /**
     * @brief GPS reference time of last noise addition.
     */
    oem_gpsec           noiseTime;

    /**
     * @brief GPS reference time of last update.
     */
    oem_gpsec           updateTime;

    /**
     * @brief Clock correction parameters.
     */
    oem_double          parameters[3];

    /**
     * @brief Covariance of the straight line fit (a 3x3 array).
     */
    oem_double          covData[9];

    /**
     * @brief Last instantaneous measurement of the range bias (m).
     */
    oem_double          rangeBias;

    /**
     * @brief Last instantaneous measurement of the range bias rate (m).
     */
    oem_double          rangeBiasRate;

    /** @brief unused. */
    oem_bool            reserved[4];

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_clockmodel;

#endif
