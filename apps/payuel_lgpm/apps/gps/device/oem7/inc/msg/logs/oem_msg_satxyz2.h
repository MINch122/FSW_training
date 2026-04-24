#ifndef _OEM_MSG_SATXYZ2_H_
#define _OEM_MSG_SATXYZ2_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_SATXYZ2 1451

/**
 * @brief SATXYZ2 log component. See oem_log_satxyz2.
 */
typedef struct {
    /**
     * @brief Satellite system. See Table 102.
     */
    oem_enum            system;

    /**
     * @brief Satellite ID.
     */
    oem_ulong           satelliteID;

    /**
     * @brief Satellite ECEF X coordinates (m).
     */
    oem_double          X;

    /**
     * @brief Satellite ECEF Y coordinates (m).
     */
    oem_double          Y;

    /**
     * @brief Satellite ECEF Z coordinates (m).
     */
    oem_double          Z;

    /**
     * @brief Satellite clock correction (m).
     */
    oem_double          clkCorr;

    /**
     * @brief Ionosphere delay (m).
     */
    oem_double          ionoDelay;

    /**
     * @brief Troposphere delay (m).
     */
    oem_double          tropoDelay;

    /** @brief Unused. */
    oem_double          Reserved1;
    /** @brief Unused. */
    oem_double          Reserved2;

} OEM_PACK oem_log_satxyz2_comp;

/**
 * @struct oem_log_satxyz2
 *
 * @brief Satellite positions in ECEF Cartesian coordinates. Synch log. MID = 1451.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief Number of satellites to follow.
     */
    oem_ulong           numSat;

    /**
     * @brief Component per observation.
     */
    oem_log_satxyz2_comp comp[];

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_satxyz2;

#endif