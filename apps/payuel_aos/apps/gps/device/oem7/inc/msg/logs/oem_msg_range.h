#ifndef _OEM_MSG_RANGE_H_
#define _OEM_MSG_RANGE_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_RANGE 43

/**
 * @brief RANGE log component. See oem_log_range.
 */
typedef struct {
    /**
     * @brief Satellite PRN number of range measurement.
     */
    oem_ushort          prnSlot;

    /**
     * @brief GLONASS Frequency + 7.
     */
    oem_ushort          gloFreq;

    /**
     * @brief Pseudorange measurement (m).
     */
    oem_double          psr;

    /**
     * @brief Pseudorange measurement standard deviation (m).
     */
    oem_float           psrStd;

    /**
     * @brief Carrier phase (cycle).
     */
    oem_double          adr;

    /**
     * @brief Estimated carrier phase standard deviation (cycle).
     */
    oem_float           adrStd;

    /**
     * @brief Instantaneous carrier Doppler frequency (Hz).
     */
    oem_float           dopp;

    /**
     * @brief Carrier to noise density ratio.
     */
    oem_float           CN0;

    /**
     * @brief Number of seconds of continuous tracking.
     */
    oem_float           locktime;

    /**
     * @brief Channel tracking status. See Table 123.
     */
    oem_ulong           chTrStatus;

} OEM_PACK oem_log_range_comp;

/**
 * @struct oem_log_range
 *
 * @brief Satellite range information. Synch log. MID = 43.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief # of observations with information to follow.
     */
    oem_ulong           numObs;

    /**
     * @brief Component per observation.
     */
    oem_log_range_comp  comp[];

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_range;

#endif